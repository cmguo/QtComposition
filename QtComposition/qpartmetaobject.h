#ifndef JSMETAOBJECT_H
#define JSMETAOBJECT_H

#include <QMetaObject>
#include <QVariant>

class QPartMetaObject : public QMetaObject
{
public:
    QPartMetaObject(QString const & desc);

    ~QPartMetaObject();
};

#endif // JSMETAOBJECT_H
