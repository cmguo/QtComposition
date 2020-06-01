#ifndef QIMPORT_H
#define QIMPORT_H

#include "QtComposition_global.h"
#include "qpart.h"
#include "qlazy.h"

#include <QVector>
#include <vector>
#include <list>

class QExportBase;
class QComponentRegistry;

class QTCOMPOSITION_EXPORT QImportBase : public QPart
{
public:
    enum Import
    {
        optional,
        exactly,
        many
    };

public:
    QImportBase(QMetaObject const * meta, char const * prop);

    template<typename T>
    inline static void registerType()
    {
        qRegisterMetaType<T*>();
    }

    template<typename T>
    class TypeT : public Type
    {
    public:
        TypeT() : Type(&T::staticMetaObject) { }
        void apply(QPart & p) const
        {
            Type::apply(p);
            static_cast<QImportBase&>(p).typeRegister_ = &registerType<T>;
        }
    };

    template<>
    class TypeT<void> : public Type
    {
    public:
        TypeT() : Type(nullptr) { }
    };

    template<typename U, typename List>
    inline static bool registerImportManyConverter()
    {
        qRegisterMetaType<List>();
        return QMetaType::registerConverter<QVector<QObject*>, List>([](QVector<QObject*> const & f) {
            List list;
            for (auto l : f)
                list.push_back(qobject_cast<U*>(l));
            return list;
        });
    }

    template<typename U>
    inline static void registerImportManyConverters()
    {
        static bool ok = registerImportManyConverter<U, std::list<U*>>()
            && registerImportManyConverter<U, std::vector<U*>>()
            && registerImportManyConverter<U, QList<U*>>()
            && registerImportManyConverter<U, QVector<U*>>();
        (void) ok;
    }

    template<typename T>
    class TypeMT : public Type
    {
    public:
        TypeMT() : Type(&T::staticMetaObject) { }
        void apply(QPart & p) const
        {
            Type::apply(p);
            static_cast<QImportBase&>(p).typeRegister_ = &registerImportManyConverters<T>;
        }
    };

    class Lazy
    {
    public:
        Lazy(bool lazy) : lazy_(lazy) {}
        void apply(QPart & p) const
        {
            static_cast<QImportBase &>(p).lazy_ = lazy_;
        }
    private:
        bool lazy_;
    };

    class Imports
    {
    public:
        Imports(Import count) : count_(count) {}
        void apply(QPart & p) const
        {
            static_cast<QImportBase &>(p).count_ = count_;
        }
    private:
        Import count_;
    };

public:
    bool valid() const;

    void compose(QObject * obj, QObject * target) const;

    void compose(QObject * obj, QVector<QObject *> const & targets) const;

    void compose(QObject * obj, QLazy target) const;

    void compose(QObject * obj, QVector<QLazy> const & targets) const;

private:
    friend class QComponentRegistry;
    char const * prop_;
    Import count_;
    bool lazy_;
    QVector<QExportBase const *> exports;
    void (*typeRegister_)();
};

template <typename T, typename U>
class QImport : QImportBase
{
public:
    QImport(char const * prop, Share share = Share::any, bool lazy = false)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U>(), Shared(share), Lazy(lazy));
    }

    QImport(char const * prop, char const * name, Share share = Share::any, bool lazy = false)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U>(), Name(name), Shared(share), Lazy(lazy));
    }

    template <typename ...Args>
    QImport(char const * prop, Args const & ...args)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U>(), args...);
    }
};

template <typename T, typename U>
class QImportMany : QImportBase
{
public:
    QImportMany(char const * prop, Share share = Share::any, bool lazy = false)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeMT<U>(), Shared(share), Lazy(lazy), Imports(many));
    }

    QImportMany(char const * prop, char const * name, Share share = Share::any, bool lazy = false)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeMT<U>(), Name(name), Shared(share), Lazy(lazy), Imports(many));
    }
};

template <typename T, typename U>
class QImportOptional : QImportBase
{
public:
    QImportOptional(char const * prop, Share share = Share::any, bool lazy = false)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U>(), Shared(share), Lazy(lazy), Imports(optional));
    }

    QImportOptional(char const * prop, char const * name, Share share = Share::any, bool lazy = false)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U>(), Name(name), Shared(share), Lazy(lazy), Imports(optional));
    }

};

#endif // QIMPORT_H
