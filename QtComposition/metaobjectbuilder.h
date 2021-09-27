#ifndef METAOBJECTBUILD_H
#define METAOBJECTBUILD_H

#include "QtComposition_global.h"

#include <QHash>
#include <QMap>
#include <QMetaMethod>
#include <QVector>

class QTCOMPOSITION_EXPORT MetaObjectBuilder
{
public:
    MetaObjectBuilder();

    void setClassName(QByteArray const & className);

    void addClassInfo(QByteArray const & key, QByteArray const & value);

    void addConstructor(const QByteArray &prototype);

    void addConstructor(const QByteArray &prototype, const QByteArray &parameters);

    void addSignal(const QByteArray &prototype);

    void addSignal(const QByteArray &prototype, const QByteArray &parameters);

    void addSignal(const QMetaMethod & method);

    void addSlot(const QByteArray &prototype, int flags);

    void addSlot(const QByteArray &type, const QByteArray &prototype, const QByteArray &parameters, int flags);

    void addSlot(const QMetaMethod & method);

    void addProperty(const QByteArray &type, const QByteArray &name, uint flags);

    void addProperty(const QMetaProperty & method);

    void addSetterSlot(const QByteArray &property);

    void addEnumValue(const QByteArray &enumname, const QByteArray &key, int value);

    void buildMetaData(QMetaObject * meta);

protected:
    bool hasClassInfo(const char *key)
    {
        return classinfos_.contains(key);
    }

    bool hasSignal(const QByteArray &prototype)
    {
        return signals_.contains(prototype);
    }

    bool hasSlot(const QByteArray &prototype)
    {
        return slots_.contains(prototype);
    }

    bool hasProperty(const QByteArray &name)
    {
        return properties_.contains(name);
    }

    bool hasEnum(const QByteArray &enumname)
    {
        return enums_.contains(enumname);
    }

protected:
    struct Property
    {
        Property() = default;
        QByteArray type;
        uint flags = 0;
        QByteArray realType;
    };

    struct Method
    {
        QByteArray type;
        QByteArray parameters;
        int flags = 0;
        QByteArray realPrototype;
    };

    int aggregateParameterCount(const QMap<QByteArray, Method> &map);

private:
    int numParameter(const QByteArray &prototype);

    QByteArray paramType(const QByteArray &signature, int index, bool *out = nullptr);

    QByteArray propertyType(const QByteArray &propertyName)
    {
        return realPrototype_.value(propertyName);
    }

    void parsePrototype(const QByteArray &prototype);

private:
    using ByteArrayIntPair = QPair<QByteArray, int>;
    using ByteArrayIntPairList = QList<ByteArrayIntPair>;

    QByteArray classname_;
    QMap<QByteArray, QByteArray> classinfos_;
    QMap<QByteArray, ByteArrayIntPairList> enums_;
    QMap<QByteArray, Property> properties_;
    QMap<QByteArray, Method> signals_;
    QMap<QByteArray, Method> slots_;

    // Prototype -> member info
    QHash<QByteArray, QList<QByteArray> > memberInfo_;
    QMap<QByteArray, QByteArray> realPrototype_;
};

#endif // METAOBJECTBUILD_H
