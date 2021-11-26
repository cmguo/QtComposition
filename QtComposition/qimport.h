#ifndef QIMPORT_H
#define QIMPORT_H

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

protected:
    QImportBase(QMetaObject const * meta, char const * prop);

    QImportBase(QVariantMap const & desc);

    template<typename T>
    inline static void registerType()
    {
        qRegisterMetaType<T*>();
    }

    template<typename T, typename Unused>
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

    template<typename Unused>
    class TypeT<void, Unused> : public Type
    {
    public:
        TypeT() : Type(nullptr) { }
    };

    template<typename Unused>
    class TypeT<void*, Unused> : public Type
    {
    public:
        TypeT() : Type(AUTO_META) { }
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

    template<typename T, typename Unused>
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

    template<typename Unused>
    class TypeMT<void, Unused> : public Type
    {
    public:
        TypeMT() : Type(nullptr) { }
    };

    template<typename Unused>
    class TypeMT<void*, Unused> : public Type
    {
    public:
        TypeMT() : Type(AUTO_META) { }
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
    bool checkType();

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

template <typename T, typename U = void*>
class QImport : QImportBase
{
public:
    QImport(char const * prop, Share share = Share::any)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U, void>(), Shared(share));
    }

    QImport(char const * prop, char const * name, Share share = Share::any)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U, void>(), Name(name), Shared(share));
    }

    template <typename ...Args, is_config_t<Args...> = true>
    QImport(char const * prop, Args const & ...args)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U, void>(), args...);
    }
};

template <typename T, typename U = void*>
class QImportMany : QImportBase
{
public:
    QImportMany(char const * prop, Share share = Share::any)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeMT<U, void>(), Shared(share), Imports(many));
    }

    QImportMany(char const * prop, char const * name, Share share = Share::any)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeMT<U, void>(), Name(name), Shared(share), Imports(many));
    }
};

template <typename T, typename U = void*>
class QImportOptional : QImportBase
{
public:
    QImportOptional(char const * prop, Share share = Share::any)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U, void>(), Shared(share), Imports(optional));
    }

    QImportOptional(char const * prop, char const * name, Share share = Share::any)
        : QImportBase(&T::staticMetaObject, prop)
    {
        config(TypeT<U, void>(), Name(name), Shared(share), Imports(optional));
    }

};

#endif // QIMPORT_H
