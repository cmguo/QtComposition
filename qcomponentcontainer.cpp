#include "qcomponentcontainer.h"
#include "qcomponentregistry.h"

#include <vector>
#include <list>

QComponentContainer & QComponentContainer::globalInstance()
{
    static QComponentContainer c;
    return c;
}

template <typename T>
static void registerConverters() {
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
}

QComponentContainer::QComponentContainer()
{
    QComponentRegistry::composition();
    registerConverters<QLazy>();
    registerConverters<QObject*>();
}

QObject * QComponentContainer::get_export_value(QPart const & i, QPart const & e)
{
    return get_export_value(*e.meta_, e.share(i));
}

QObject * QComponentContainer::get_export_value(QLazy const & lazy)
{
    return get_export_value(*lazy.part_->meta_, lazy.share_);
}

QObject * QComponentContainer::get_export_value(QMetaObject const & meta, bool share)
{
    return get_export_value(meta, share, [](auto meta) {
        return meta.newInstance();
    });
}

QObject * QComponentContainer::get_export_value(
        QMetaObject const & meta, bool share,
        std::function<QObject *(QMetaObject const &)> const & creator)
{
    QObject * o = nullptr;
    if (share) {
        auto it = shared_objs_.find(&meta);
        if (it == shared_objs_.end()) {
            o = creator(meta);
            QComponentRegistry::compose(this, meta, o);
            shared_objs_.insert(std::make_pair(&meta, o));
            meta.invokeMethod(o, "onComposition");
        } else {
            o = it->second;
        }
    } else {
        temp_non_shared_objs_.push_back(QVector<QObject *>());
        o = creator(meta);
        QComponentRegistry::compose(this, meta, o);
        auto it = non_shared_objs_.insert(std::make_pair(o, QVector<QObject *>()));
        it.first->second.swap(temp_non_shared_objs_.back());
        temp_non_shared_objs_.pop_back();
        if (!temp_non_shared_objs_.empty())
            temp_non_shared_objs_.back().push_back(o);
        meta.invokeMethod(o, "onComposition");
    }
    return o;
}

QObject * QComponentContainer::get_export_value(QPart const & i)
{
    auto exports = QComponentRegistry::get_exports(i);
    if (exports.empty())
        return nullptr;
    if (exports.size() > 1)
        return nullptr;
    return get_export_value(i, *exports.front());
}

QObject * QComponentContainer::get_export_value(QMetaObject const & meta, QPart::Share share)
{
    return get_export_value(QPart(&meta, &meta, meta.className(), share));
}

QObject * QComponentContainer::get_export_value(char const * name, QPart::Share share)
{
    return get_export_value(QPart(nullptr, nullptr, name, share));
}

QVector<QObject *> QComponentContainer::get_export_values(QPart const & i)
{
    auto exports = QComponentRegistry::get_exports(i);
    QVector<QObject *> list;
    for (auto e : exports)
        list.push_back(get_export_value(i, *e));
    return list;
}

QVector<QObject *> QComponentContainer::get_export_values(QMetaObject const & meta, QPart::Share share)
{
    return get_export_values(QPart(&meta, &meta, meta.className(), share));
}

QVector<QObject *> QComponentContainer::get_export_values(char const * name, QPart::Share share)
{
    return get_export_values(QPart(nullptr, nullptr, name, share));
}

void QComponentContainer::release_value(QObject *value)
{
    auto it = non_shared_objs_.find(value);
    if (it == non_shared_objs_.end())
        return;
    for (auto o : it->second)
        release_value(o);
    non_shared_objs_.erase(it);
    value->deleteLater();
}

QLazy QComponentContainer::get_export(QPart const & i)
{
    auto exports = QComponentRegistry::get_exports(i);
    if (exports.empty())
        return QLazy();
    if (exports.size() > 1)
        return QLazy();
    QExportBase const * e = exports.front();
    return QLazy(this, e, e->share(i));
}

QLazy QComponentContainer::get_export(QMetaObject const & meta, QPart::Share share)
{
    return get_export(QPart(&meta, &meta, meta.className(), share));
}

QLazy QComponentContainer::get_export(char const * name, QPart::Share share)
{
    return get_export(QPart(nullptr, nullptr, name, share));
}

QVector<QLazy> QComponentContainer::get_exports(QPart const & i)
{
    auto exports = QComponentRegistry::get_exports(i);
    QVector<QLazy> list;
    for (auto e : exports)
        list.push_back(QLazy(this, e, e->share(i)));
    return list;
}

QVector<QLazy> QComponentContainer::get_exports(QMetaObject const & meta, QPart::Share share)
{
    return get_exports(QPart(&meta, &meta, meta.className(), share));
}

QVector<QLazy> QComponentContainer::get_exports(char const * name, QPart::Share share)
{
    return get_exports(QPart(nullptr, nullptr, name, share));
}

