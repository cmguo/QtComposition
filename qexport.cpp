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

void QExportBase::collectClassInfo()
{
    QMap<char const *, char const *> attrs;
    for (int i = 0; i < meta_->classInfoCount(); ++i) {
        QMetaClassInfo const ci = meta_->classInfo(i);
        attrs.insert(ci.name(), ci.value());
    }
    if (attrs.isEmpty())
        return;
    if (!attrs_.isEmpty()) {
        auto i = attrs_.keyValueBegin();
        for (; i != attrs_.keyValueEnd(); ++i) {
            attrs.insert((*i).first, (*i).second);
        }
    }
    attrs.swap(attrs_);
}

