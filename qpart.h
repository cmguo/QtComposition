#ifndef QPART_H
#define QPART_H

#include <QMetaObject>

class QPart
{
public:
    enum Share
    {
        any,
        shared,
        nonshared
    };

    enum Import
    {
        optional,
        exactly,
        many
    };

public:
    QPart(QMetaObject const * meta, QMetaObject const * type, char const * name, Share share = Share::any);

protected:
    QPart(QMetaObject const * meta, QMetaObject const * type, char const * name, Share share, bool isExport);

    bool match(QPart const & o) const;

    bool share(QPart const & o) const;

    char const * name() const
    {
        return name_ == nullptr ? type_->className() : name_;
    }

protected:
    friend class QComponentRegistry;
    friend class QComponentContainer;
    QMetaObject const * meta_;
    QMetaObject const * type_;
    char const * name_;
    Share share_;
};

#endif // QPART_H
