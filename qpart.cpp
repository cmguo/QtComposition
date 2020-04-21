#include "qpart.h"
#include "qcomponentregistry.h"

#include <string.h>

QPart::QPart(QMetaObject const * meta, QMetaObject const * type, char const * name, Share share)
    : meta_(meta)
    , type_(type)
    , name_(name)
    , share_(share)
{
}

QPart::QPart(QMetaObject const * meta, bool isExport)
    : meta_(meta)
    , type_(meta)
    , name_(nullptr)
    , share_(any)
{
    if (isExport)
        QComponentRegistry::add_export(static_cast<QExportBase *>(this));
    else
        QComponentRegistry::add_import(static_cast<QImportBase *>(this));
}

QPart::QPart(const QPart &o, const QMetaObject *newType)
    : meta_(o.meta_)
    , type_(newType)
    , name_(nullptr)
    , share_(o.share_)
    , attrs_(o.attrs_)
{
}

bool QPart::match(const QPart &i) const
{
    return (type_ == i.type_ || i.type_ == nullptr) && strcmp(name(), i.name()) == 0
            && (share_ == any || i.share_ == any || share_ == i.share_)
            && attrMatch(i);
}

bool QPart::attrMatch(const QPart &i) const
{
    auto iter = i.attrs_.begin();
    for (; iter != i.attrs_.end(); ++iter) {
        char const * my = attrs_.value(iter.key());
        if (my == nullptr || strcmp(my, *iter))
            return false;
    }
    return true;
}

bool QPart::share(const QPart &i) const
{
    return i.share_ != Share::nonshared && share_ != Share::nonshared;
}

