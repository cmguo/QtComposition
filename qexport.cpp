#include "qexport.h"

#include <string.h>

#include "qcomponentregistry.h"

QExportBase::QExportBase(QMetaObject const * meta)
    : QPart(meta, true)
{
}

