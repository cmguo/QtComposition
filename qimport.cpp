#include "qimport.h"

#include <QObject>
#include <QMetaProperty>
#include <QVariant>

QImportBase::QImportBase(QMetaObject const * meta, char const * prop)
    : QPart(meta, false)
    , prop_(prop)
    , count_(Import::exactly)
    , lazy_(false)
    , typeRegister_(nullptr)
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
    if (typeRegister_)
        typeRegister_();
    obj->setProperty(prop_, QVariant::fromValue(target));
}

void QImportBase::compose(QObject * obj, QVector<QObject *> const & targets) const
{
    if (typeRegister_)
        typeRegister_();
    obj->setProperty(prop_, QVariant::fromValue(targets));
}

void QImportBase::compose(QObject * obj, QLazy target) const
{
    if (typeRegister_)
        typeRegister_();
    obj->setProperty(prop_, QVariant::fromValue(target));
}

void QImportBase::compose(QObject * obj, QVector<QLazy> const & targets) const
{
    if (typeRegister_)
        typeRegister_();
    obj->setProperty(prop_, QVariant::fromValue(targets));
}
