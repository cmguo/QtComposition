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
    QComponentRegistry::composition();
    static bool ok = registerConverters<QLazy>()
            && registerConverters<QObject*>();
    (void) ok;
}

QComponentContainer::~QComponentContainer()
{
    for (QObject * so : sharedObjs_)
        delete so;
    sharedObjs_.clear();
    for (void * nso : nonSharedObjs_.keys()) {
        qDebug() << "may leak not shared object " << nso;
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
            QComponentRegistry::compose(this, meta, o);
            sharedObjs_.insert(&meta, o);
            int index = meta.indexOfMethod("onComposition()");
            if (index >= 0)
                meta.method(index).invoke(o);
        } else {
            o = *it;
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
    if (it != sharedObjs_.end() && *it == value)
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

