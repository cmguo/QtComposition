#include "qimport.h"

#include <QObject>
#include <QMetaProperty>
#include <QVariant>

QImportBase::QImportBase(QMetaObject const * meta, char const * prop)
    : QPart(meta, meta, nullptr, Share::any, false)
    , prop_(prop)
    , count_(Import::exactly)
    , lazy_(false)
{
}

QImportBase::QImportBase(QMetaObject const * meta, char const * prop,
                         QMetaObject const * type)
    : QPart(meta, type, nullptr, Share::any, false)
    , prop_(prop)
    , count_(Import::exactly)
    , lazy_(false)
{
}

QImportBase::QImportBase(QMetaObject const * meta, char const * prop,
                         Share share, bool lazy,
                         QMetaObject const * type, char const * name)
    : QPart(meta, type, name, share, false)
    , prop_(prop)
    , count_(Import::exactly)
    , lazy_(lazy)
{
}

bool QImportBase::valid() const
{
    if (count_ == Import::exactly)
        return exports.size() == 1;
    else if (count_ == Import::optional)
        return exports.size() <= 1;
    else
        return true;
}

void QImportBase::compose(QObject * obj, QObject * target) const
{
    obj->setProperty(prop_, QVariant::fromValue(target));
}

void QImportBase::compose(QObject * obj, std::vector<QObject *> const & targets) const
{
    obj->setProperty(prop_, QVariant::fromValue(targets));
}

void QImportBase::compose(QObject * obj, QLazy target) const
{
    obj->setProperty(prop_, QVariant::fromValue(target));
}

void QImportBase::compose(QObject * obj, std::vector<QLazy> const & targets) const
{
    obj->setProperty(prop_, QVariant::fromValue(targets));
}
