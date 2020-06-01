#ifndef QCOMPONENTREGISTRY_H
#define QCOMPONENTREGISTRY_H

#include "QtComposition_global.h"
#include "qexport.h"
#include "qimport.h"

#include <QVector>
#include <QMap>

class QComponentContainer;

class QComponentRegistry
{
public:
    static void addExport(QExportBase * e);

    static void addImport(QImportBase * e);

    static void composition();

    static QVector<QExportBase const *> collectExports(QPart const & i);

    static QVector<QExportBase const *> getExports(QImportBase const & i);

    static void compose(QComponentContainer * cont, QMetaObject const & type, QObject * obj);

private:
    struct Meta;

    static Meta & getMeta(QMetaObject const * type);

    static QMap<QMetaObject const *, Meta> metas_;
};

#endif // QCOMPONENTREGISTRY_H
