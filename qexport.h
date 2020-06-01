#ifndef QEXPORT_H
#define QEXPORT_H

#include "QtComposition_global.h"
#include "qpart.h"

class QImportBase;
class QComponentRegistry;
class QComponentContainer;

class QTCOMPOSITION_EXPORT QExportBase : public QPart
{
protected:
    QExportBase(QMetaObject const * meta);

    QExportBase(QExportBase const & o, QMetaObject const * newType);

private:
    friend class QComponentRegistry;
    friend class QComponentContainer;
};

template <typename T, typename U = T>
class QExport : QExportBase
{
public:
    QExport(Share share = Share::any)
        : QExportBase(&T::staticMetaObject)
    {
        config(Type(&U::staticMetaObject), Shared(share));
    }

    QExport(char const * name, Share share = Share::any)
        : QExportBase(&T::staticMetaObject)
    {
        config(Type(&U::staticMetaObject), Name(name), Shared(share));
    }

    template <typename ...Args>
    QExport(Args const & ...args)
        : QExportBase(&T::staticMetaObject)
    {
        config(Type(&U::staticMetaObject), args...);
    }

};

#endif // QEXPORT_H
