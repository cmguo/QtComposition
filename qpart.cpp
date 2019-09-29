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

QPart::QPart(QMetaObject const * meta, QMetaObject const * type, char const * name, Share share, bool isExport)
    : meta_(meta)
    , type_(type)
    , name_(name)
    , share_(share)
{
    if (isExport)
        QComponentRegistry::add_export(static_cast<QExportBase *>(this));
    else
        QComponentRegistry::add_import(static_cast<QImportBase *>(this));
}

bool QPart::match(const QPart &i) const
{
    return (type_ == i.type_ || i.type_ == nullptr) && strcmp(name(), i.name()) == 0
            && (share_ == any || i.share_ == any || share_ == i.share_);
}

bool QPart::share(const QPart &i) const
{
    return i.share_ != Share::nonshared && share_ != Share::nonshared;
}

