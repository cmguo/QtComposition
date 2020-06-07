#ifndef QPART_H
#define QPART_H

#include "QtComposition_global.h"

#include <QMetaObject>
#include <QMap>

#define QPART_ATTR_MINE_TYPE "mineType"

#define Q_MINE_TYPE(x) Q_CLASSINFO("mineType", x)

#define Q_INHERITED_EXPORT Q_CLASSINFO("InheritedExport", "true")

class QTCOMPOSITION_EXPORT QPart
{
public:
    enum Share
    {
        any,
        shared,
        nonshared
    };

    static char const * ATTR_MINE_TYPE;

public:
    QPart(QMetaObject const * meta, QMetaObject const * type, char const * name, Share share = Share::any);

    class Type
    {
    public:
        Type(QMetaObject const * type) : type_(type) {}
        void apply(QPart & p) const
        {
            p.type_ = type_;
        }
    private:
        QMetaObject const * type_;
    };

    class Name
    {
    public:
        Name(char const * name) : name_(name) {}
        void apply(QPart & p) const
        {
            p.name_ = name_;
        }
    private:
        char const * name_;
    };

    class Shared
    {
    public:
        Shared(Share share) : share_(share) {}
        void apply(QPart & p) const
        {
            p.share_ = share_;
        }
    private:
        Share share_;
    };

    class Attribute
    {
    public:
        Attribute(char const * key, char const * value) : key_(key), value_(value) {}
        void apply(QPart & p) const
        {
            p.attrs_[key_] = value_;
        }
    private:
        char const * key_;
        char const * value_;
    };

    class MineTypeAttribute : public Attribute
    {
    public:
        MineTypeAttribute(char const * value) : Attribute(ATTR_MINE_TYPE, value) {}
    };

protected:
    QPart(QMetaObject const * meta, bool isExport);

    QPart(QPart const & o, QMetaObject const * newType);

    static QMetaObject const * const AUTO_META;

    template <typename Arg, typename ...Args>
    void config(Arg const & arg, Args const & ...args)
    {
        arg.apply(*this);
        config(args...);
    }

    void config()
    {
    }

    bool match(QPart const & o) const;

    bool attrMatch(QPart const & o) const;

    bool share(QPart const & o) const;

public:
    QMetaObject const * meta() const
    {
        return meta_;
    }

    QMetaObject const * type() const
    {
        return type_;
    }

    char const * name() const
    {
        return name_ == nullptr ? type_->className() : name_;
    }

    Share share() const
    {
        return share_;
    }

    char const * attr(char const * key, char const * defalutValue = nullptr) const;

    char const * attrMineType() const { return attr(ATTR_MINE_TYPE); }

protected:
    friend class QComponentRegistry;
    friend class QComponentContainer;
    QMetaObject const * meta_;
    QMetaObject const * type_;
    char const * name_;
    Share share_;
    QMap<char const *, char const *> attrs_;
};

#endif // QPART_H
