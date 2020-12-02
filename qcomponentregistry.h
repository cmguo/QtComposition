#ifndef QCOMPONENTREGISTRY_H
#define QCOMPONENTREGISTRY_H

#include "QtComposition_global.h"
#include "qexport.h"
#include "qimport.h"

#include <QVector>
#include <QMap>

class QComponentContainer;

class QTCOMPOSITION_EXPORT QComponentRegistry
{
public:
    static void addExport(QExportBase * e);

    static void addImport(QImportBase * i);

    static void importPlugins(QString const & path);

    static void composition();

    static void importPlugin(QString const & file);

    static void exportPlugin(QString const & file, QString const & json);

    static QMetaObject const & loadMeta(QMetaObject const & meta2);

    static QMetaObject const & unloadMeta(QMetaObject const & meta2);

    static QVector<QExportBase const *> collectExports(QPart const & i);

    static QVector<QExportBase const *> getExports(QImportBase const & i);

    static void compose(QComponentContainer * cont, QMetaObject const & type, QObject * obj);

    static void compose(QComponentContainer * cont, QMetaObject const & type, QObject * obj, QVector<QObject *> & depends);

private:
    struct Meta;
    struct Loader;

    static Meta & getMeta(QMetaObject const * type);

    static void inheritedExport(Meta & m, QExportBase * e);

    static void overrideExport(Meta & m);

    static void attachPart(Meta & m, QPart * p, bool isExport);

    static void detachPart(Meta & m, QPart * p, bool isExport);

private:
    static QMap<QMetaObject const *, Meta> metas_;
    static QList<Loader*> loaders_;
    static QMap<QPart *, bool> foundParts_;
    static bool composed_;
};

#endif // QCOMPONENTREGISTRY_H
