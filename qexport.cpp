#include "qexport.h"

#include <QMetaClassInfo>
#include <string.h>

#include "qcomponentregistry.h"

QExportBase::QExportBase(QMetaObject const * meta)
    : QPart(meta, true)
{
}

QExportBase::QExportBase(const QExportBase &o, const QMetaObject *newType)
    : QPart(o, newType)
{
}

QExportBase::QExportBase(const QVariantMap &desc)
    : QPart(desc, true)
{
}

void QExportBase::collectClassInfo()
{
    QMap<char const *, char const *> attrs;
    for (int i = 0; i < meta_->classInfoCount(); ++i) {
        QMetaClassInfo const ci = meta_->classInfo(i);
        if (strcmp(ci.name(), "InheritedExport") == 0
                || strcmp(ci.name(), "OverrideExport") == 0
                || strcmp(ci.name(), "version") == 0)
            continue;
        attrs.insert(ci.name(), ci.value());
    }
    if (attrs.isEmpty())
        return;
    if (!attrs_.isEmpty()) {
        auto i = attrs_.begin();
        for (; i != attrs_.end(); ++i) {
            attrs.insert(i.key(), i.value());
        }
    }
    attrs.swap(attrs_);
}

