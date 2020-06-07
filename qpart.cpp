#include "qpart.h"
#include "qcomponentregistry.h"

#include <string.h>

char const * QPart::ATTR_MINE_TYPE = QPART_ATTR_MINE_TYPE;

QMetaObject const * const QPart::AUTO_META = reinterpret_cast<QMetaObject*>(1);

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
        QComponentRegistry::addExport(static_cast<QExportBase *>(this));
    else
        QComponentRegistry::addImport(static_cast<QImportBase *>(this));
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

const char * QPart::attr(const char *key, const char *defalutValue) const
{
    const char * value = attrs_.value(key);
    if (value)
        return value;
    auto i = attrs_.keyValueBegin();
    for (; i != attrs_.keyValueEnd(); ++i) {
        if (strcmp(key, (*i).first) == 0)
            return (*i).second;
    }
    return defalutValue;
}
