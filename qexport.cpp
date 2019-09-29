#include "qexport.h"

#include <string.h>

#include "qcomponentregistry.h"

QExportBase::QExportBase(QMetaObject const * meta)
    : QPart(meta, meta, nullptr, Share::any, true)
{
}

QExportBase::QExportBase(QMetaObject const * meta, QMetaObject const * type)
    : QPart(meta, type, nullptr, Share::any, true)
{
}

QExportBase::QExportBase(QMetaObject const * meta, QMetaObject const * type,
                         char const * name, Share share)
    : QPart(meta, type, name, share, true)
{
}

