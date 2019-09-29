#include "qtestcomposition.h"

#include "qexport.h"
#include "qimport.h"

static QImport<QTestComposition, QObject> import_qtest_impl("impl");
static QImportMany<QTestComposition, QObject> import_qtest_impls("impls", QPart::nonshared, true);
static QExport<QTestComposition> export_qtest;

QTestComposition::QTestComposition(QObject *parent) : QObject(parent)
{

}

void QTestComposition::on_composition()
{
    for (auto l : impls_)
        l.get<QObject>();
}
