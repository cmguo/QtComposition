#ifndef QEXPORT_H
#define QEXPORT_H

#include "qpart.h"

class QImportBase;
class QComponentRegistry;
class QComponentContainer;

class QTCOMPOSITION_EXPORT QExportBase : public QPart
{
protected:
    QExportBase(QMetaObject const * meta);

    QExportBase(QVariantMap const & desc);

    QExportBase(QExportBase const & o, QMetaObject const * newType);

private:
    friend class QComponentRegistry;
    friend class QComponentContainer;

    void collectClassInfo();
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

    template <typename ...Args, is_config_t<Args...> = true>
    QExport(Args const & ...args)
        : QExportBase(&T::staticMetaObject)
    {
        config(Type(&U::staticMetaObject), args...);
    }

};

#endif // QEXPORT_H
