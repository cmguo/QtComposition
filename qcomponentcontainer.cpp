#include "qcomponentcontainer.h"
#include "qcomponentregistry.h"

#include <QMetaMethod>
#include <QDebug>

#include <vector>
#include <list>

QComponentContainer & QComponentContainer::globalInstance()
{
    static QComponentContainer c;
    return c;
}

template <typename T>
static bool registerConverters() {
    qRegisterMetaType<std::list<T>>();
    qRegisterMetaType<std::vector<T>>();
    qRegisterMetaType<QList<T>>();
    qRegisterMetaType<QVector<T>>();
    QMetaType::registerConverter<QVector<T>, std::list<T>>([](QVector<T> const & f) {
        return f.toList().toStdList();
    });
    QMetaType::registerConverter<QVector<T>, std::vector<T>>([](QVector<T> const & f) {
        return f.toStdVector();
    });
    QMetaType::registerConverter<QVector<T>, QList<T>>([](QVector<T> const & f) {
        return f.toList();
    });
    return true;
}

QComponentContainer::QComponentContainer()
{
    static bool ok = registerConverters<QLazy>()
            && registerConverters<QObject*>();
    (void) ok;
    QComponentRegistry::composition();
}

QComponentContainer::~QComponentContainer()
{
    QVector<QObject *> deleted;
    do {
        QVector<QObject *> nd;
        for (QVector<QObject *> & so : sharedObjs_) {
            if (so.size() > 1)
                so.erase(std::remove_if(so.begin(), so.end(),
                                    [deleted](QObject * o) { return deleted.contains(o); }),
                    so.end());
            if (so.size() == 1) {
                nd.append(so.first());
                qDebug() << "release shared object " << so.first();
                delete so.first();
                so.clear();
            }
        }
        deleted.swap(nd);
    } while (!deleted.isEmpty());
    for (QVector<QObject *> & so : sharedObjs_) {
        if (!so.isEmpty()) {
            qWarning() << "can't release shared object " << so.first();
        }
    }
    sharedObjs_.clear();
    for (void * nso : nonSharedObjs_.keys()) {
        qWarning() << "may leak not shared object " << nso;
    }
}

QObject * QComponentContainer::getExportValue(QPart const & i, QPart const & e)
{
    return getExportValue(*e.meta_, e.share(i));
}

QObject * QComponentContainer::getExportValue(QLazy const & lazy)
{
    return getExportValue(*lazy.part_->meta_, lazy.share_);
}

QObject * QComponentContainer::getExportValue(QMetaObject const & meta, bool share)
{
    return getExportValue(meta, share, [](auto meta) {
        return meta.newInstance();
    });
}

QObject * QComponentContainer::getExportValue(
        QMetaObject const & meta, bool share,
        std::function<QObject *(QMetaObject const &)> const & creator)
{
    QObject * o = nullptr;
    if (share) {
        auto it = sharedObjs_.find(&meta);
        if (it == sharedObjs_.end()) {
            o = creator(meta);
            if (o == nullptr) {
                qWarning() << "QComponentContainer failed create object of " << meta.className();
                return nullptr;
            }
            o->setParent(this);
            it = sharedObjs_.insert(&meta, QVector<QObject*>{o});
            QVector<QObject *> depends;
            QComponentRegistry::compose(this, meta, o, depends);
            for (QObject * d : depends) {
                auto it = sharedObjs_.find(d->metaObject());
                if (it != sharedObjs_.end() && (*it).first() == d)
                    (*it).append(o);
            }
            int index = meta.indexOfMethod("onComposition()");
            if (index >= 0)
                meta.method(index).invoke(o);
        } else {
            o = (*it).first();
        }
    } else {
        //temp_non_shared_objs_.push_back(QVector<QObject *>());
        o = creator(meta);
        if (o == nullptr) {
            qWarning() << "QComponentContainer failed create object of " << meta.className();
            return nullptr;
        }
        QComponentRegistry::compose(this, meta, o);
        //auto it = non_shared_objs_.insert(o, QVector<QObject *>());
        //it->swap(temp_non_shared_objs_.back());
        //temp_non_shared_objs_.pop_back();
        //if (!temp_non_shared_objs_.empty())
        //    temp_non_shared_objs_.back().push_back(o);
        int index = meta.indexOfMethod("onComposition()");
        if (index >= 0)
            meta.method(index).invoke(o);
    }
    return o;
}

QObject * QComponentContainer::getExportValue(QPart const & i)
{
    auto exports = QComponentRegistry::collectExports(i);
    if (exports.empty())
        return nullptr;
    if (exports.size() > 1)
        return nullptr;
    return getExportValue(i, *exports.front());
}

QObject *QComponentContainer::getExportValue(const QImportBase &i)
{
    auto exports = QComponentRegistry::getExports(i);
    if (exports.empty())
        return nullptr;
    if (exports.size() > 1)
        return nullptr;
    return getExportValue(i, *exports.front());
}

QObject * QComponentContainer::getExportValue(QMetaObject const & meta, QPart::Share share)
{
    return getExportValue(QPart(&meta, &meta, meta.className(), share));
}

QObject * QComponentContainer::getExportValue(char const * name, QPart::Share share)
{
    return getExportValue(QPart(nullptr, nullptr, name, share));
}

QVector<QObject *> QComponentContainer::getExportValues(QPart const & i)
{
    auto exports = QComponentRegistry::collectExports(i);
    QVector<QObject *> list;
    for (auto e : exports)
        list.push_back(getExportValue(i, *e));
    return list;
}

QVector<QObject *> QComponentContainer::getExportValues(const QImportBase &i)
{
    auto exports = QComponentRegistry::getExports(i);
    QVector<QObject *> list;
    for (auto e : exports)
        list.push_back(getExportValue(i, *e));
    return list;
}

void QComponentContainer::composeValue(QObject *value)
{
    QComponentRegistry::compose(this, *value->metaObject(), value);
}

QVector<QObject *> QComponentContainer::getExportValues(QMetaObject const & meta, QPart::Share share)
{
    return getExportValues(QPart(&meta, &meta, meta.className(), share));
}

QVector<QObject *> QComponentContainer::getExportValues(char const * name, QPart::Share share)
{
    return getExportValues(QPart(nullptr, nullptr, name, share));
}

void QComponentContainer::releaseValue(QObject *value)
{
    auto it = sharedObjs_.find(value->metaObject());
    if (it != sharedObjs_.end() && (*it).first() == value)
        return;
    //auto it = non_shared_objs_.find(value);
    //if (it == non_shared_objs_.end())
    //    return;
    //for (auto o : (*it))
    //    release_value(o);
    //non_shared_objs_.erase(it);
    value->deleteLater();
}

QLazy QComponentContainer::getExport(QPart const & i)
{
    auto exports = QComponentRegistry::collectExports(i);
    if (exports.empty())
        return QLazy();
    if (exports.size() > 1)
        return QLazy();
    QExportBase const * e = exports.front();
    return QLazy(this, e, e->share(i));
}

QLazy QComponentContainer::getExport(const QImportBase &i)
{
    auto exports = QComponentRegistry::getExports(i);
    if (exports.empty())
        return QLazy();
    if (exports.size() > 1)
        return QLazy();
    QExportBase const * e = exports.front();
    return QLazy(this, e, e->share(i));
}

QLazy QComponentContainer::getExport(QMetaObject const & meta, QPart::Share share)
{
    return getExport(QPart(&meta, &meta, meta.className(), share));
}

QLazy QComponentContainer::getExport(char const * name, QPart::Share share)
{
    return getExport(QPart(nullptr, nullptr, name, share));
}

QVector<QLazy> QComponentContainer::getExports(QPart const & i)
{
    auto exports = QComponentRegistry::collectExports(i);
    QVector<QLazy> list;
    for (auto e : exports)
        list.push_back(QLazy(this, e, e->share(i)));
    return list;
}

QVector<QLazy> QComponentContainer::getExports(const QImportBase &i)
{
    auto exports = QComponentRegistry::getExports(i);
    QVector<QLazy> list;
    for (auto e : exports)
        list.push_back(QLazy(this, e, e->share(i)));
    return list;
}

QVector<QLazy> QComponentContainer::getExports(QMetaObject const & meta, QPart::Share share)
{
    return getExports(QPart(&meta, &meta, meta.className(), share));
}

QVector<QLazy> QComponentContainer::getExports(char const * name, QPart::Share share)
{
    return getExports(QPart(nullptr, nullptr, name, share));
}

