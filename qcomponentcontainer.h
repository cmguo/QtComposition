#ifndef QCOMPONENTCONTAINER_H
#define QCOMPONENTCONTAINER_H

#include "QtComposition_global.h"
#include "qcomponentregistry.h"
#include "qimport.h"
#include "qlazy.h"

#include <functional>

class QTCOMPOSITION_EXPORT QComponentContainer
{
public:
    static QComponentContainer & globalInstance();

public:
    QComponentContainer();

    ~QComponentContainer();

public:
    template<typename T>
    T * getExportValue(QPart::Share share = QPart::Share::any)
    {
        return qobject_cast<T *>(getExportValue(T::staticMetaObject, share));
    }

    QObject * getExportValue(QMetaObject const & type, QPart::Share share = QPart::Share::any);

    QObject * getExportValue(char const * name, QPart::Share share = QPart::Share::any);

    QObject * getExportValue(QPart const & i);

    QObject * getExportValue(QImportBase const & i);

    template<typename T>
    QVector<QObject *> getExportValues(QPart::Share share = QPart::Share::any)
    {
        return getExportValues(T::staticMetaObject, share);
    }

    QVector<QObject *> getExportValues(QMetaObject const & type, QPart::Share share = QPart::Share::any);

    QVector<QObject *> getExportValues(char const * name, QPart::Share share = QPart::Share::any);

    QVector<QObject *> getExportValues(QPart const & i);

    QVector<QObject *> getExportValues(QImportBase const & i);

    void composeValue(QObject * value);

    void releaseValue(QObject * value);

public:
    template<typename T>
    QLazy getExport(QPart::Share share)
    {
        return getExport(T::staticMetaObject, share);
    }

    QLazy getExport(QMetaObject const & type, QPart::Share share);

    QLazy getExport(char const * name, QPart::Share share);

    QLazy getExport(QPart const & i);

    QLazy getExport(QImportBase const & i);

    template<typename T>
    QVector<QLazy> getExports(QPart::Share share)
    {
        return getExports(T::staticMetaObject, share);
    }

    QVector<QLazy> getExports(QMetaObject const & type, QPart::Share share);

    QVector<QLazy> getExports(char const * name, QPart::Share share);

    QVector<QLazy> getExports(QPart const & i);

    QVector<QLazy> getExports(QImportBase const & i);

    QObject * getExportValue(QLazy const & lazy);

    typedef std::function<QObject *(QMetaObject const &)> creator_t;

    template<typename ...Args>
    QObject * getExportValue(QLazy const & lazy, Args&&... args)
    {
        return getExportValue(*lazy.part_->meta_, false, [args...](QMetaObject const & meta) {
            return meta.newInstance(std::move(args)...);
        });
    }

private:
    Q_DISABLE_COPY(QComponentContainer)

    QObject * getExportValue(QPart const & i, QPart const & e);

    QObject * getExportValue(QMetaObject const & meta, bool share, creator_t const & creator);

    QObject * getExportValue(QMetaObject const & meta, bool share);

private:
    QMap<QMetaObject const *, QVector<QObject *>> sharedObjs_;
    QMap<QObject *, QVector<QObject *>> nonSharedObjs_;
    QVector<QVector<QObject *>> tempNonSharedObjs_;
};

#endif // QCOMPONENTCONTAINER_H
