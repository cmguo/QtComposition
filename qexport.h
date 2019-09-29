#ifndef QEXPORT_H
#define QEXPORT_H

#include "QtComposition_global.h"
#include "qpart.h"

class QImportBase;
class QComponentRegistry;
class QComponentContainer;

class QTCOMPOSITION_EXPORT QExportBase : public QPart
{
public:
    QExportBase(QMetaObject const * meta);

    QExportBase(QMetaObject const * meta, QMetaObject const * type);

    QExportBase(QMetaObject const * meta, QMetaObject const * type, char const * name, Share share = Share::any);

private:
    friend class QComponentRegistry;
    friend class QComponentContainer;
};

template <typename T, typename U = T>
class QExport : QExportBase
{
public:
    QExport(Share share = Share::any)
        : QExportBase(
              &T::staticMetaObject,
              &U::staticMetaObject,
              nullptr, share)
    {
    }

    QExport(char const * name, Share share = Share::any)
        : QExportBase(
              &T::staticMetaObject,
              &U::staticMetaObject,
              name, share)
    {
    }

};

#endif // QEXPORT_H
