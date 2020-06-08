#include "qcomponentregistry.h"
#include "qcomponentcontainer.h"

#include <QDebug>
#include <QMetaClassInfo>

struct QComponentRegistry::Meta
{
    Meta() : state(Unknown){}

    enum State { Unknown, Valid, Invalid };
    State state;
    QVector<QExportBase *> exports;
    QVector<QImportBase *> imports;
};

QMap<QMetaObject const *, QComponentRegistry::Meta> QComponentRegistry::metas_;

void QComponentRegistry::addExport(QExportBase * e)
{
    getMeta(e->meta_).exports.push_back(e);
}

void QComponentRegistry::addImport(QImportBase * i)
{
    getMeta(i->meta_).imports.push_back(i);
}

void QComponentRegistry::composition()
{
    static bool composed = false;
    if (composed == true)
        return;
    composed = true;
    // InheritedExport
    for (auto m = metas_.keyValueBegin(); m != metas_.keyValueEnd(); ++m) {
        QVector<QExportBase *> exports = (*m).second.exports;
        for (QExportBase * i : exports) {
            i->collectClassInfo();
            QMetaObject const * type = (i->type_ ? i->type_ : i->meta_)->superClass();
            while (type && type != &QObject::staticMetaObject) {
                int index = type->indexOfClassInfo("InheritedExport");
                if (index >= type->superClass()->classInfoCount()) {
                    if (QByteArray("true") == type->classInfo(index).value()) {
                        (*m).second.exports.push_back(new QExportBase(*i, type));
                    }
                }
                type = type->superClass();
            }
        }
    }
    QVector<QMetaObject const *> invalids;
    int count = 0;
    for (auto m = metas_.keyValueBegin(); m != metas_.keyValueEnd(); ++m) {
        (*m).second.state = Meta::Valid;
        for (QImportBase * i : (*m).second.imports) {
            if (!i->checkType()) {
                qWarning() << "QComponentRegistry:" << (*m).first->className() << "!import" << i->prop_;
                (*m).second.state = Meta::Invalid;
                invalids.push_back((*m).first);
                break;
            }
            i->exports = collectExports(*i);
            if (!i->valid()) {
                qWarning() << "QComponentRegistry:" << (*m).first->className() << "!import" << i->prop_;
                (*m).second.state = Meta::Invalid;
                invalids.push_back((*m).first);
                break;
            }
        }
    }
    while (count < invalids.size()) {
        QMetaObject const * meta = invalids[count++];
        for (auto m = metas_.keyValueBegin(); m != metas_.keyValueEnd(); ++m) {
            if ((*m).second.state == Meta::Invalid)
                continue;
            if ((*m).first->inherits(meta)) {
                qWarning() << "QComponentRegistry:" << (*m).first->className() << "!base" << meta->className();
                (*m).second.state = Meta::Invalid;
                invalids.push_back((*m).first);
                continue;
            }
            for (QImportBase * i : (*m).second.imports) {
                i->exports.erase(
                            std::remove_if(i->exports.begin(), i->exports.end(),
                                           [meta](auto e) { return e->meta_ == meta;}),
                            i->exports.end());
                if (!i->valid()) {
                    qWarning() << "QComponentRegistry:" << (*m).first->className() << "!import" << i->prop_;
                    (*m).second.state = Meta::Invalid;
                    invalids.push_back((*m).first);
                    break;
                }
            }
        }
    }
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

QComponentRegistry::Meta & QComponentRegistry::getMeta(QMetaObject const * meta)
{
    auto iter = metas_.find(meta);
    if (iter == metas_.end()) {
        iter = metas_.insert(meta, Meta());
    }
    return *iter;
}

QVector<QExportBase const *> QComponentRegistry::collectExports(QPart const & i)
{
    QVector<QExportBase const *> list;
    for (auto & m : metas_) {
        if (m.state == Meta::Invalid)
            continue;
        for (QExportBase * e : m.exports) {
            if (e->match(i)) {
                list.push_back(e);
                break;
            }
        }
    }
    return list;
}

QVector<const QExportBase *> QComponentRegistry::getExports(const QImportBase &i)
{
    return i.exports;
}
