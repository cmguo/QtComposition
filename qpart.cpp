#include "qpart.h"
#include "qcomponentregistry.h"
#include "qpartmetaobject.h"

#include <QMetaClassInfo>

#include <stdexcept>

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

QPart::QPart(const QVariantMap &desc, bool isExport)
    : meta_(nullptr)
    , type_(nullptr)
    , name_(nullptr)
    , share_(any)
{
    meta_ = buildMetaObject(desc.value("meta").toString());
    if (desc.contains("type"))
        type_ = buildMetaObject(desc.value("type").toString());
    if (desc.contains("name"))
        name_ = registerString(desc.value("name").toString().toUtf8());
    if (desc.contains("share")) {
        static QStringList shareNames = {"any", "shared", "nonshared"};
        int share = shareNames.indexOf(desc.value("share").toString());
        if (share < 0)
            throw std::runtime_error("qpart share error");
        share_ = static_cast<Share>(share);
    }
    if (desc.contains("attrs")) {
        QVariantMap attrs = desc.value("attrs").toMap();
        for (auto it = attrs.begin(); it != attrs.end(); ++it)
            attrs_.insert(registerString(it.key().toUtf8()),
                          registerString(it.value().toString().toUtf8()));
    }
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
    return typeMatch(type_, i.type_)
            && strcmp(name(), i.name()) == 0
            && (share_ == any || i.share_ == any || share_ == i.share_)
            && attrMatch(attrs_, i.attrs_);
}

bool QPart::share(const QPart &i) const
{
    return i.share_ != Share::nonshared && share_ != Share::nonshared;
}

const char *QPart::name() const
{
    return name_ == nullptr ? type_->className() : name_;
}

const char * QPart::attr(const char *key, const char *defalutValue) const
{
    const char * value = attrs_.value(key);
    if (value)
        return value;
    auto i = attrs_.begin();
    for (; i != attrs_.end(); ++i) {
        if (strcmp(key, i.key()) == 0)
            return i.value();
    }
    return defalutValue;
}

bool QPart::typeMatch(const QMetaObject *me, const QMetaObject *mi)
{
    if (me == mi || mi == nullptr)
        return true;
    if (me == nullptr)
        return false;
    if (strcmp(me->className(), mi->className()) != 0)
        return false;
    int ve = me->indexOfClassInfo("version");
    int vi = mi->indexOfClassInfo("version");
    if (vi < 0 && ve < 0)
        return true;
    if (ve >= 0 && vi >= 0
            && strcmp(me->classInfo(ve).value(), mi->classInfo(vi).value()) == 0)
        return true;
    return false;
}

bool QPart::attrMatch(const QMap<const char *, const char *> &ae, const QMap<const char *, const char *> &ai)
{
    auto iter = ai.begin();
    for (; iter != ai.end(); ++iter) {
        char const * my = ae.value(iter.key());
        if (my == nullptr) {
            auto i = ae.begin();
            for (; i != ae.end(); ++i) {
                if (strcmp(iter.key(), i.key()) == 0) {
                    my = i.value();
                    break;
                }
            }
        }
        if (my == nullptr || (*iter != nullptr && strcmp(my, *iter)))
            return false;
    }
    return true;
}

const QMetaObject *QPart::buildMetaObject(const QString & desc)
{
    static QMap<QString, QMetaObject const *> metas;
    QMetaObject const * meta = metas.value(desc);
    if (meta)
        return meta;
    meta = new QPartMetaObject(desc);
    metas.insert(desc, meta);
    return meta;
}

const char *QPart::registerString(const QByteArray &str)
{
    static QList<QByteArray> strings;
    int istr = strings.indexOf(str);
    if (istr < 0) {
        istr = strings.count();
        strings.append(str);
    }
    return strings[istr].data();
}
