#include "qexport.h"

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

