#ifndef QCOMPONENTCONTAINER_H
#define QCOMPONENTCONTAINER_H

#include "QtComposition_global.h"
#include "qcomponentregistry.h"
#include "qimport.h"
#include "qlazy.h"

#include <functional>

class QTCOMPOSITION_EXPORT QComponentContainer
{
public:
    static QComponentContainer & globalInstance();

public:
    QComponentContainer();

public:
    template<typename T>
    T * get_export_value(QPart::Share share = QPart::Share::any)
    {
        return qobject_cast<T *>(get_export_value(T::staticMetaObject, share));
    }

    QObject * get_export_value(QMetaObject const & type, QPart::Share share = QPart::Share::any);

    QObject * get_export_value(char const * name, QPart::Share share = QPart::Share::any);

    QObject * get_export_value(QPart const & i);

    template<typename T>
    std::vector<QObject *> get_export_values(QPart::Share share = QPart::Share::any)
    {
        return get_export_values(T::staticMetaObject, share);
    }

    std::vector<QObject *> get_export_values(QMetaObject const & type, QPart::Share share = QPart::Share::any);

    std::vector<QObject *> get_export_values(char const * name, QPart::Share share = QPart::Share::any);

    std::vector<QObject *> get_export_values(QPart const & i);

    void release_value(QObject * value);

public:
    template<typename T>
    QLazy get_export(QPart::Share share)
    {
        return get_export(T::staticMetaObject, share);
    }

    QLazy get_export(QMetaObject const & type, QPart::Share share);

    QLazy get_export(char const * name, QPart::Share share);

    QLazy get_export(QPart const & i);

    template<typename T>
    std::vector<QLazy> get_exports(QPart::Share share)
    {
        return get_exports(T::staticMetaObject, share);
    }

    std::vector<QLazy> get_exports(QMetaObject const & type, QPart::Share share);

    std::vector<QLazy> get_exports(char const * name, QPart::Share share);

    std::vector<QLazy> get_exports(QPart const & i);

    QObject * get_export_value(QLazy const & lazy);

    typedef std::function<QObject *(QMetaObject const &)> creator_t;

    template<typename ...Args>
    QObject * get_export_value(QLazy const & lazy, Args&&... args)
    {
        return get_export_value(*lazy.part_->meta_, false, [this, args...](QMetaObject const & meta) {
            return meta.newInstance(std::move(args)...);
        });
    }

private:
    QObject * get_export_value(QPart const & i, QPart const & e);

    QObject * get_export_value(QMetaObject const & meta, bool share, creator_t const & creator);

    QObject * get_export_value(QMetaObject const & meta, bool share);

private:
    std::map<QMetaObject const *, QObject *> shared_objs_;
    std::map<QObject *, std::vector<QObject *>> non_shared_objs_;
    std::vector<std::vector<QObject *>> temp_non_shared_objs_;
};

#endif // QCOMPONENTCONTAINER_H
