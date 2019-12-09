#ifndef QCOMPONENTREGISTRY_H
#define QCOMPONENTREGISTRY_H

#include "QtComposition_global.h"
#include "qexport.h"
#include "qimport.h"

#include <QVector>
#include <map>

class QComponentContainer;

class QComponentRegistry
{
public:
    static void add_export(QExportBase * e);

    static void add_import(QImportBase * e);

    static void composition();

    static QVector<QExportBase const *> get_exports(QPart const & i);

    static void compose(QComponentContainer * cont, QMetaObject const & type, QObject * obj);

private:
    struct Meta;

    static Meta & get_meta(QMetaObject const * type);

    static std::map<QMetaObject const *, Meta> metas_;
};

#endif // QCOMPONENTREGISTRY_H
