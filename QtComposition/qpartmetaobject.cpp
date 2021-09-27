#include "qpartmetaobject.h"
#include "metaobjectbuilder.h"

#include <QByteArray>
#include <QList>
#include <QMetaMethod>
#include <QObject>

QPartMetaObject::QPartMetaObject(QString const & desc)
{
    d.data = nullptr;
    d.stringdata = nullptr;
    MetaObjectBuilder builder;
    int n = desc.indexOf('/');
    if (n > 0) {
        builder.setClassName(desc.left(n).toUtf8());
        builder.addClassInfo("version", desc.mid(n + 1).toUtf8());
    } else {
        builder.setClassName(desc.toUtf8());
    }
    builder.buildMetaData(this);
}

QPartMetaObject::~QPartMetaObject()
{
    delete [] d.data;
    delete [] reinterpret_cast<char *>(const_cast<QByteArrayData *>(d.stringdata));
}

