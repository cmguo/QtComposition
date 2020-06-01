#include "qtestcomposition.h"

#include "qexport.h"
#include "qimport.h"

static QImport<QTestComposition, QObject> import_qtest_impl("impl");
static QImportMany<QTestComposition, QObject> import_qtest_impls("impls", QPart::nonshared);
static QExport<QTestComposition> export_qtest;
static QExport<QObject> export_qobject(QPart::shared);

QTestComposition::QTestComposition(QObject *parent) : QObject(parent)
{

}

void QTestComposition::onComposition()
{
    for (auto l : impls_)
        l.get<QObject>();
}
