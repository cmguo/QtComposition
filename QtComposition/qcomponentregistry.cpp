#include "qcomponentregistry.h"
#include "qcomponentcontainer.h"
#include "qcomponentfactoryinterface.h"

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QMetaClassInfo>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QJsonDocument>

#include <QtWidgets/QApplication>

struct QComponentRegistry::Loader : public QPluginLoader
{
    QList<QString> depends;
    QList<Meta *> metas; // loaded metas
    QMap<QPart *, bool> parts;
    // TODO: when can we unload
};

struct QComponentRegistry::Meta
{
    enum State { Unknown, Valid, Invalid };
    // not always equal metas_'s key (fake meta)
    QMetaObject const * meta = nullptr;
    State state = Unknown;
    QVector<QExportBase *> exports;
    QVector<QImportBase *> imports;
    QVector<QMetaObject const *> overrides;
    // for plugin
    Loader * loader = nullptr;
    // Record parts for unloadMeta
    QMap<QPart *, QPart *> attaches;
};

QMap<QMetaObject const *, QComponentRegistry::Meta> QComponentRegistry::metas_;
// use for static register parts, and then use by loadMeta after composition
QMap<QPart *, bool> QComponentRegistry::foundParts_;
QMap<QString, QComponentRegistry::Loader*> QComponentRegistry::loaders_;
bool QComponentRegistry::composed_ = false;

void QComponentRegistry::addExport(QExportBase * e)
{
    foundParts_.insert(e, true);
}

void QComponentRegistry::addImport(QImportBase * i)
{
    foundParts_.insert(i, false);
}

void QComponentRegistry::importPlugins(const QString &path)
{
    QDir dir(path);
    for (QString const & f : dir.entryList(QDir::Files)) {
        if (!QLibrary::isLibrary(f))
            continue;
        importPlugin(dir.filePath(f));
    }
}

void QComponentRegistry::composition()
{
    if (composed_ == true)
        return;
    composed_ = true;
    // Collect loaders from plugins
    QString compDir = qApp->applicationDirPath() + "/plugins/components";
    importPlugins(compDir);
    // Collect Parts
    for (auto it = foundParts_.begin(); it != foundParts_.end(); ++it) {
        Meta & m = getMeta(it.key()->meta());
        if (it.value()) { // this is export part
            QExportBase * e = static_cast<QExportBase *>(it.key());
            // Translate classInfo into attrs
            e->collectClassInfo();
            // Handle "InheritedExport": also export with super type
            inheritedExport(m, e);
            m.exports.append(e);
        } else {
            m.imports.append(static_cast<QImportBase *>(it.key()));
        }
    }
    foundParts_.clear();
    // Collect Loader's Metas, which are fake metaobjects really
    for (Loader * l : loaders_) {
        QSet<QMetaObject const *> metas;
        for (auto it = l->parts.begin(); it != l->parts.end(); ++it) {
            metas.insert(it.key()->meta());
        }
        l->parts.clear();
        for (QMetaObject const * m : metas) {
            metas_[m].loader = l;
        }
    }
    // Verify (also with plugin meta and parts)
    QVector<QMetaObject const *> invalids;
    int count = 0;
    for (auto & m : metas_) {
        m.state = Meta::Valid;
        for (QImportBase * i : m.imports) {
            if (!i->checkType()) {
                qWarning() << "QComponentRegistry:" << m.meta->className() << "!import" << i->prop_;
                m.state = Meta::Invalid;
                invalids.push_back(m.meta);
                break;
            }
            i->exports = collectExports(*i);
            if (!i->valid()) {
                qWarning() << "QComponentRegistry:" << m.meta->className() << "!import" << i->prop_;
                m.state = Meta::Invalid;
                invalids.push_back(m.meta);
                break;
            }
        }
    }
    // Verify Recursive
    while (count < invalids.size()) {
        QMetaObject const * meta = invalids[count++];
        for (auto & m : metas_) {
            if (m.state == Meta::Invalid)
                continue;
            // Invalidate children
            if (m.meta->inherits(meta)) {
                qWarning() << "QComponentRegistry:" << m.meta->className() << "!base" << meta->className();
                m.state = Meta::Invalid;
                invalids.push_back(m.meta);
                continue;
            }
            // Invalidate imports
            for (QImportBase * i : m.imports) {
                i->exports.erase(
                            std::remove_if(i->exports.begin(), i->exports.end(),
                                           [meta](auto e) { return e->meta_ == meta;}),
                            i->exports.end());
                if (!i->valid()) {
                    qWarning() << "QComponentRegistry:" << m.meta->className() << "!import" << i->prop_;
                    m.state = Meta::Invalid;
                    invalids.push_back(m.meta);
                    break;
                }
            }
        }
    }
}

void QComponentRegistry::importPlugin(const QString &file)
{
    qInfo() << "QComponentRegistry::importPlugin" << file;
    Loader * l = new Loader;
    l->setFileName(file);
    QJsonObject meta = l->metaData(); // created by MetaObjectBuilder
    if (meta.value("IID").toString() != ComponentFactory_iid) {
        delete l;
        return;
    }
    meta = meta.value("MetaData").toObject();
    QString name = meta.value("Name").toString();
    QJsonArray depends = meta.value("Depends").toArray();
    for (QJsonValueRef d : depends) {
        if (d.isString()) {
            l->depends.append(d.toString());
        }
    }
    QJsonArray exports = meta.value("Exports").toArray();
    for (QJsonValueRef e : exports) {
        if (e.isObject()) {
            try {
                QExportBase * e1 = new QExportBase(e.toObject().toVariantMap());
                qDebug() << "QComponentRegistry::importPlugin" << e1->meta()->className()
                         << e1->type()->className() << e1->name();
                l->parts.insert(e1, true);
            } catch (std::exception & e) {
                qWarning() << "QComponentRegistry::importPlugin" << file << e.what();
            }
        }
    }
    QJsonArray imports = meta.value("Imports").toArray();
    for (QJsonValueRef i : imports) {
        if (i.isObject()) {
            try {
                QImportBase * i1 = new QImportBase(i.toObject().toVariantMap());
                qDebug() << "QComponentRegistry::importPlugin" << i1->meta()->className()
                         << i1->prop_ << i1->type()->className();
                l->parts.insert(i1, false);
            } catch (std::exception & e) {
                qWarning() << "QComponentRegistry::importPlugin" << file << e.what();
            }
        }
    }
    if (name.isEmpty()) {
        name = QFileInfo(file).completeBaseName();
#ifdef WIN32
#ifdef _DEBUG
        if (name.endsWith("d"))
            name.truncate(name.size() - 1);
#endif
#endif
    }
    loaders_.insert(name, l);
}

void QComponentRegistry::exportPlugin(const QString &file, const QString &json)
{
    QMap<QMetaObject const *, QString> metaNames;
    auto metaName = [&metaNames] (QMetaObject const * meta) {
        QString n = metaNames.value(meta);
        if (!n.isEmpty())
            return n;
        n = meta->className();
        int i = meta->indexOfClassInfo("version");
        if (i >= 0)
            n = n + "/" + meta->classInfo(i).value();
        return n;
    };
    static QStringList countNames = {"optional", "exactly", "many"};
    static QStringList shareNames = {"any", "shared", "nonshared"};
    auto fillPart = [metaName] (QJsonObject & ex, QPart * p) {
        ex.insert("meta", metaName(p->meta()));
        ex.insert("type", metaName(p->type()));
        if (p->name_)
            ex.insert("name", p->name_);
        if (p->share_ != QPart::any)
            ex.insert("share", shareNames[p->share_]);
        if (!p->attrs_.isEmpty()) {
            QJsonObject attrs;
            for (auto it = p->attrs_.begin(); it != p->attrs_.end(); ++it)
                attrs.insert(it.key(), it.value());
            ex.insert("attrs", attrs);
        }
    };
    QLibrary lib(file);
    lib.load();
    lib.unload();
    QJsonArray Keys;
    QJsonArray Exports;
    QJsonArray Imports;
    QSet<QMetaObject const *> metas;
    for (auto it = foundParts_.begin(); it != foundParts_.end(); ++it) {
        if (!metas.contains(it.key()->meta())) {
            Keys.append(metaName(it.key()->meta()));
            metas.insert(it.key()->meta());
        }
        if (it.value()) {
            QExportBase * e = static_cast<QExportBase*>(it.key());
            e->collectClassInfo();
            QJsonObject ex;
            fillPart(ex, e);
            Exports.append(ex);
        } else {
            QImportBase * i = static_cast<QImportBase*>(it.key());
            QJsonObject im;
            i->checkType();
            fillPart(im, i);
            im.insert("prop", i->prop_);
            im.insert("count", countNames[i->count_]);
            im.insert("lazy", i->lazy_);
            Imports.append(im);
        }
    }
    foundParts_.clear();
    QJsonObject root;
    root.insert("Keys", Keys);
    root.insert("Exports", Exports);
    root.insert("Imports", Imports);
    QJsonDocument doc(root);
    QFile f(json);
    f.open(QFile::WriteOnly);
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
}

const QMetaObject & QComponentRegistry::loadMeta(const QMetaObject &meta2)
{
    // Fake metaobjects can't have static_metacall
    if (meta2.d.static_metacall || !metas_.contains(&meta2))
        return meta2;
    auto & m = metas_[&meta2];
    if (m.loader == nullptr) {
        qWarning() << "QComponentRegistry::loadMeta no loader";
        return meta2;
    }
    if (!loadPlugin(m.loader)) {
        return meta2;
    }
    qInfo() << "QComponentRegistry::loadMeta" << meta2.className();
    for (auto it = m.loader->parts.begin(); it != m.loader->parts.end(); ++it) {
        QPart * p = it.key(); // this is real part
        if (m.meta == p->meta() || QPart::typeMatch(m.meta, p->meta())) {
            // Replace with real metaobject
            if (m.meta != p->meta()) {
                m.meta = p->meta();
                overrideExport(m); // collect all super meta
            }
            // Prepare parts and swap with fake one
            attachPart(m, p, it.value());
        }
    }
    if (m.meta == &meta2) {
        qWarning() << "QComponentRegistry::loadMeta no match parts";
        return meta2;
    }
    m.loader->metas.append(&m);
    return *m.meta;
}

const QMetaObject &QComponentRegistry::unloadMeta(const QMetaObject &meta2)
{
    for (auto & m : metas_) {
        // Real metaobject equal
        if (m.meta == &meta2) {
            if (m.loader == nullptr)
                return meta2;
            for (auto it = m.attaches.begin(); it != m.attaches.end(); ++it) {
                detachPart(m, it.key(), m.exports.contains(
                               static_cast<QExportBase*>(it.value())));
            }
            if (!m.attaches.isEmpty())
                m.meta = m.attaches.first()->meta();
            m.attaches.clear();
            m.loader->metas.removeAll(&m);
            if (m.loader->metas.isEmpty()) {
                m.loader->unload();
                m.loader->parts.clear();
            }
            return *m.meta;
        }
    }
    return meta2;
}

void QComponentRegistry::compose(QComponentContainer *cont, const QMetaObject &type, QObject *obj)
{
    QVector<QObject *> depends;
    compose(cont, type, obj, depends);
}

void QComponentRegistry::compose(
        QComponentContainer * cont, QMetaObject const & type, QObject * obj, QVector<QObject *> & depends)
{
    if (&type == &QObject::staticMetaObject)
        return;
    auto iter = metas_.find(&type);
    if (iter == metas_.end())
        iter = std::find_if(metas_.begin(), metas_.end(),
                            [&type] (Meta & m) { return m.meta == &type; });
    if (iter == metas_.end()) {
        compose(cont, *type.superClass(), obj, depends);
        return;
    }
    Meta const & meta = *iter;
    for (auto i : meta.imports) {
        if (i->count_ == QImportBase::many) {
            if (i->lazy_) {
                i->compose(obj, cont->getExports(*i));
            } else {
                QVector<QObject *> deps = cont->getExportValues(*i);
                if (i->share() != QPart::nonshared)
                    depends.append(deps);
                i->compose(obj, deps);
            }
        } else {
            if (i->lazy_) {
                i->compose(obj, cont->getExport(*i));
            } else {
                QObject * dep = cont->getExportValue(*i);
                if (i->share() != QPart::nonshared)
                    depends.append(dep);
                i->compose(obj, dep);
            }
        }
    }
    compose(cont, *type.superClass(), obj, depends);
}

QVector<const QExportBase *> QComponentRegistry::getAllExports(QPart::Share share)
{
    QVector<QExportBase const *> list;
    QPart i(nullptr, nullptr, nullptr, share);
    for (auto & m : metas_) {
        if (m.state == Meta::Invalid)
            continue;
        for (QExportBase * e : m.exports) {
            if (e->match(i))
                list.append(e);
        }
    }
    return list;
}

QComponentRegistry::Meta & QComponentRegistry::getMeta(QMetaObject const * meta)
{
    auto iter = metas_.find(meta);
    if (iter == metas_.end()) {
        iter = metas_.insert(meta, Meta());
        (*iter).meta = meta;
        overrideExport((*iter));
    }
    return *iter;
}

void QComponentRegistry::inheritedExport(QComponentRegistry::Meta &m, QExportBase *e)
{
    // We start with my part type, not my part meta
    QMetaObject const * type = (e->type_ ? e->type_ : e->meta_)->superClass();
    while (type && type != &QObject::staticMetaObject) {
        int index = type->indexOfClassInfo("InheritedExport");
        if (index >= type->superClass()->classInfoCount()) {
            if (QByteArray("true") == type->classInfo(index).value()) {
                m.exports.push_back(new QExportBase(*e, type));
            }
        }
        type = type->superClass();
    }
}

void QComponentRegistry::overrideExport(QComponentRegistry::Meta &m)
{
    QMetaObject const * type = m.meta;
    int index = type->indexOfClassInfo("OverrideExport");
    if (index >= type->classInfoCount()
            && QByteArray("true") == type->classInfo(index).value()) {
        type = type->superClass();
        while (type && type != &QObject::staticMetaObject) {
            m.overrides.append(type);
            type = type->superClass();
        }
    }
}

bool QComponentRegistry::loadPlugin(QComponentRegistry::Loader *loader)
{
    if (!loader->isLoaded()) {
        qInfo() << "QComponentRegistry::loadPlugin" << loader->fileName();
        for (auto d : loader->depends) {
            Loader * loader2 = loaders_.value(d);
            if (loader2 == nullptr) {
                qWarning() << "QComponentRegistry::loadPlugin failed: depend" << d;
                return false;
            }
            if (!loadPlugin(loader2)) {
                return false;
            }
        }
        if (!loader->instance()) {
            qWarning() << "QComponentRegistry::loadPlugin failed" << loader->errorString();
            return false;
        }
        // When library is loadding, it's parts is register to foundParts_
        loader->parts.swap(foundParts_);
    }
    return true;
}

bool QComponentRegistry::unloadPlugin(QComponentRegistry::Loader *loader)
{
    (void) loader;
    return false;
}

void QComponentRegistry::attachPart(Meta & m, QPart *p, bool isExport)
{
    if (isExport) {
        QExportBase * ex = static_cast<QExportBase*>(p);
        ex->collectClassInfo();
        // Find fake export part
        for (QExportBase * e : m.exports) {
            if (ex->match(*ex)) {
                std::swap(*e, *ex);
                // Handle "InheritedExport": also export with super type
                inheritedExport(m, e);
                m.attaches.insert(p, e);
                return;
            }
        }
    } else {
        QImportBase * im = static_cast<QImportBase*>(p);
        im->checkType();
        // Find fake import part
        for (QImportBase * i : m.imports) {
            if (i->match(*im) && strcmp(im->prop_, i->prop_) == 0
                    && i->lazy_ == im->lazy_ && i->count_ == im->count_) {
                im->exports = i->exports;
                std::swap(*i, *im);
                m.attaches.insert(p, i);
                return;
            }
        }
    }
    qWarning() << "QComponentRegistry:attachPart:no match found"
               << p->meta()->className() << p->type()->className() << p->name();
}

void QComponentRegistry::detachPart(Meta & m, QPart *p, bool isExport)
{
    QPart * p1 = m.attaches.value(p);
    if (p1) {
        if (isExport)
            std::swap(static_cast<QExportBase&>(*p1), static_cast<QExportBase&>(*p));
        else
            std::swap(static_cast<QImportBase&>(*p1), static_cast<QImportBase&>(*p));
        return;
    }
}

QVector<QExportBase const *> QComponentRegistry::collectExports(QPart const & i)
{
    QVector<QExportBase const *> list;
    QList<QMetaObject const *> overrides;
    for (auto & m : metas_) {
        if (m.state == Meta::Invalid)
            continue;
        if (overrides.contains(m.meta))
            continue;
        for (QExportBase * e : m.exports) {
            if (!e->match(i))
                continue;
            list.push_back(e);
            // Remove overrides
            for (auto meta : m.overrides) {
                if (overrides.contains(meta))
                    continue;
                overrides.append(meta);
                list.erase(
                            std::remove_if(list.begin(), list.end(),
                                           [meta](auto e) { return e->meta_ == meta;}),
                            list.end());
            }
            break;
        }
    }
    return list;
}

QVector<const QExportBase *> QComponentRegistry::getExports(const QImportBase &i)
{
    return i.exports;
}
