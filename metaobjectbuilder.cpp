#include "metaobjectbuilder.h"

#include <QByteArray>
#include <QList>
#include <QMetaMethod>
#include <QObject>

MetaObjectBuilder::MetaObjectBuilder()
{
}

void MetaObjectBuilder::setClassName(const QByteArray &className)
{
    classname_ = className;
}

void MetaObjectBuilder::addClassInfo(const QByteArray &key, const QByteArray &value)
{
    classinfos_.insert(key, value);
}

void MetaObjectBuilder::addConstructor(const QByteArray &prototype)
{
    (void) prototype;
}

void MetaObjectBuilder::addConstructor(const QByteArray &prototype, const QByteArray &parameters)
{
    (void) prototype;
    (void) parameters;
}

static inline QList<QByteArray> paramList(const QByteArray &prototype)
{
    QByteArray parameters = prototype.mid(prototype.indexOf('(') + 1);
    parameters.truncate(parameters.length() - 1);
    if (parameters.isEmpty() || parameters == "void")
        return QList<QByteArray>();
    return parameters.split(',');
}

static const char *const type_conversion[][2] =
{
    { "float", "double"},
    { "short", "int"},
    { "char", "int"},
    { "QList<int>", "QVariantList" },
    { "QList<uint>", "QVariantList" },
    { "QList<double>", "QVariantList" },
    { "QList<bool>", "QVariantList" },
    { "QList<QDateTime>", "QVariantList" },
    { "QList<qlonglong>", "QVariantList" },
    { nullptr, nullptr }
};

inline QByteArray replaceType(const QByteArray &type)
{
    if (type.isEmpty())
        return QByteArray("void");
    int i = 0;
    while (type_conversion[i][0]) {
        int len = int(strlen(type_conversion[i][0]));
        int ti;
        if ((ti = type.indexOf(type_conversion[i][0])) != -1) {
            QByteArray rtype(type);
            rtype.replace(ti, len, type_conversion[i][1]);
            return rtype;
        }
        ++i;
    }
    return type;
}

QByteArray replacePrototype(const QByteArray &prototype)
{
   QByteArray proto(prototype);
   QList<QByteArray> plist = paramList(prototype);
   for (int p = 0; p < plist.count(); ++p) {
       const QByteArray &param = plist.at(p);
       if (param != replaceType(param)) {
           int type = 0;
           while (type_conversion[type][0]) {
               int paren = proto.indexOf('(');
               while ((paren = proto.indexOf(type_conversion[type][0])) != -1) {
                   proto.replace(paren, int(qstrlen(type_conversion[type][0])),
                                 type_conversion[type][1]);
               }
               ++type;
           }
           break;
       }
   }
   return proto;
}

void MetaObjectBuilder::addSignal(const QByteArray &prototype)
{
    int ntype = prototype.indexOf(' ');
    int nparam = prototype.indexOf('(', ntype) + 1;
    addSignal(prototype, prototype.mid(nparam, prototype.length() - nparam - 1));
}

void MetaObjectBuilder::addSignal(const QByteArray &prototype, const QByteArray &parameters)
{
    QByteArray proto(replacePrototype(prototype));
    Method &signal = signals_[proto];
    signal.type = "void";
    signal.parameters = parameters;
    signal.flags = QMetaMethod::Public | QMetaMethod::Signal;
    if (proto != prototype)
        signal.realPrototype = prototype;
}

void MetaObjectBuilder::addSlot(const QByteArray &prototype, int flags)
{
    int ntype = prototype.indexOf(' ');
    int nparam = prototype.indexOf('(', ntype) + 1;
    addSlot(prototype.left(ntype), prototype, prototype.mid(nparam, prototype.length() - nparam - 1), flags);
}

void MetaObjectBuilder::addSlot(const QByteArray &type, const QByteArray &prototype, const QByteArray &parameters, int flags)
{
    QByteArray proto = replacePrototype(prototype);
    Method &slot = slots_[proto];
    slot.type = replaceType(type);
    slot.parameters = parameters;
    slot.flags = flags | QMetaMethod::Slot;
    if (proto != prototype)
        slot.realPrototype = prototype;
}

void MetaObjectBuilder::addProperty(const QByteArray &type, const QByteArray &name, uint flags)
{
    QByteArray propertyType(type);
    if (propertyType.endsWith('&'))
        propertyType.chop(1);
    Property &prop = properties_[name];
    if (!propertyType.isEmpty()) {
        prop.type = replaceType(propertyType);
        if (prop.type != propertyType)
            prop.realType = propertyType;
    }
//    if (flags & Writable)
//        flags |= Stored;
    prop.flags |= flags;
}

void MetaObjectBuilder::addSetterSlot(const QByteArray &property)
{
    QByteArray prototype(property);
    if (isupper(prototype.at(0))) {
        prototype.insert(0, "Set");
    } else {
        prototype[0] = char(toupper(prototype[0]));
        prototype.insert(0, "set");
    }
    const QByteArray type = propertyType(property);
    if (type.isEmpty() || type == "void") {
        qWarning("%s: Invalid property '%s' of type '%s' encountered.",
                 Q_FUNC_INFO, property.constData(), type.constData());
    } else {
        prototype += '(';
        prototype += type;
        prototype += ')';
        if (!hasSlot(prototype))
            addSlot("void", prototype, property, QMetaMethod::Public);
    }
}

void MetaObjectBuilder::addEnumValue(const QByteArray &enumname, const QByteArray &key, int value)
{
    enums_[enumname].append(ByteArrayIntPair(key, value));
}

void MetaObjectBuilder::parsePrototype(const QByteArray &prototype)
{
    QByteArray realProto = realPrototype_.value(prototype, prototype);
    QByteArray parameters = realProto.mid(realProto.indexOf('(') + 1);
    parameters.truncate(parameters.length() - 1);
    if (parameters.isEmpty()) {
        memberInfo_.insert(prototype, QList<QByteArray>());
    } else {
        QList<QByteArray> plist = parameters.split(',');
        memberInfo_.insert(prototype, plist);
    }
}

static int nameToBuiltinType(const QByteArray &typeName)
{
    int id = QMetaType::type(typeName);
    return (id < QMetaType::User) ? id : QMetaType::UnknownType;
}

class QMetaStringTable
{
public:
    explicit QMetaStringTable(const QByteArray &className);
    int enter(const QByteArray &value);
    static int preferredAlignment();
    int blobSize() const;
    void writeBlob(char *out) const;
private:
    typedef QHash<QByteArray, int> Entries; // string --> index mapping
    Entries m_entries;
    int m_index;
    QByteArray m_className;
};

enum MetaDataFlags : int
{
    IsUnresolvedType = static_cast<int>(0x80000000),
    TypeNameIndexMask = 0x7FFFFFFF,
    IsUnresolvedSignal = 0x70000000
};

static uint nameToTypeInfo(const QByteArray &typeName, QMetaStringTable &strings)
{
    int id = nameToBuiltinType(typeName);
    const int result = id != QMetaType::UnknownType
        ? id : (IsUnresolvedType) | strings.enter(typeName);
    return uint(result);
}

static void writeString(char *out, int i, const QByteArray &str,
                        const int offsetOfStringdataMember, int &stringdataOffset)
{
    int size = str.size();
    qptrdiff offset = offsetOfStringdataMember + stringdataOffset
            - i * static_cast<int>(sizeof(QByteArrayData));
    const QByteArrayData data =
        Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(size, offset);
    memcpy(out + i * static_cast<int>(sizeof(QByteArrayData)), &data, sizeof(QByteArrayData));
    memcpy(out + offsetOfStringdataMember + stringdataOffset, str.constData(), static_cast<size_t>(size));
    out[offsetOfStringdataMember + stringdataOffset + size] = '\0';
    stringdataOffset += size + 1;
}

// Writes strings to string data struct.
// The struct consists of an array of QByteArrayData, followed by a char array
// containing the actual strings. This format must match the one produced by
// moc (see generator.cpp).
void QMetaStringTable::writeBlob(char *out) const
{
    Q_ASSERT(!(reinterpret_cast<quintptr>(out) & static_cast<quintptr>(preferredAlignment()-1)));
    int offsetOfStringdataMember = m_entries.size() * static_cast<int>(sizeof(QByteArrayData));
    int stringdataOffset = 0;
    // qt_metacast expects the first string in the string table to be the class name.
    writeString(out, /*index*/0, m_className, offsetOfStringdataMember, stringdataOffset);
    for (Entries::ConstIterator it = m_entries.constBegin(), end = m_entries.constEnd();
         it != end; ++it) {
        const int i = it.value();
        if (i == 0)
            continue;
        const QByteArray &str = it.key();
        writeString(out, i, str, offsetOfStringdataMember, stringdataOffset);
    }
}

struct QMetaObjectPrivate
{
    // revision 7 is Qt 5.0 everything lower is not supported
    // revision 8 is Qt 5.12: It adds the enum name to QMetaEnum
    enum { OutputRevision = 8 }; // Used by moc, qmetaobjectbuilder and qdbus
    int revision;
    int className;
    int classInfoCount, classInfoData;
    int methodCount, methodData;
    int propertyCount, propertyData;
    int enumeratorCount, enumeratorData;
    int constructorCount, constructorData;
    int flags;
    int signalCount;
};


enum { MetaObjectPrivateFieldCount = sizeof(QMetaObjectPrivate) / sizeof(int) };

void MetaObjectBuilder::buildMetaData(QMetaObject * meta)
{
    int paramsDataSize =
            ((aggregateParameterCount(signals_)
              + aggregateParameterCount(slots_)) * 2) // types and parameter names
            - signals_.count() // return "parameters" don't have names
            - slots_.count(); // ditto
    int int_data_size = MetaObjectPrivateFieldCount;
    int_data_size += classinfos_.count() * 2;
    int_data_size += (signals_.count() + slots_.count()) * 5 + paramsDataSize;
    int_data_size += properties_.count() * 3;
    int_data_size += enums_.count() * 5;
    for (auto it = enums_.cbegin(), end = enums_.cend(); it != end; ++it)
        int_data_size += it.value().count() * 2;
    ++int_data_size; // eod

    uint *int_data = new uint[static_cast<size_t>(int_data_size)];
    QMetaObjectPrivate *header = reinterpret_cast<QMetaObjectPrivate *>(int_data);
    Q_STATIC_ASSERT_X(QMetaObjectPrivate::OutputRevision == 8, "QtDBus meta-object generator should generate the same version as moc");
    header->revision = QMetaObjectPrivate::OutputRevision;
    header->className = 0;
    header->classInfoCount = classinfos_.count();
    header->classInfoData = MetaObjectPrivateFieldCount;
    header->methodCount = signals_.count() + slots_.count();
    header->methodData = header->classInfoData + header->classInfoCount * 2;
    header->propertyCount = properties_.count();
    header->propertyData = header->methodData + header->methodCount * 5 + paramsDataSize;
    header->enumeratorCount = enums_.count();
    header->enumeratorData = header->propertyData + header->propertyCount * 3;
    header->constructorCount = 0;
    header->constructorData = 0;
    header->flags = 0;
    header->signalCount = signals_.count();

    QByteArray classNameForMetaObject = classname_;
    QMetaStringTable strings(classNameForMetaObject);

    int offset = MetaObjectPrivateFieldCount;

    // each class info in form key\0value\0
    for (auto it = classinfos_.cbegin(), cend = classinfos_.cend(); it != cend; ++it) {
        int_data[offset++] = uint(strings.enter(it.key()));
        int_data[offset++] = uint(strings.enter(it.value()));
    }
    Q_ASSERT(offset == header->methodData);

    int paramsOffset = offset + (signals_.count() + slots_.count()) * 5;
    // add each method:
    for (int x = 0; x < 2; ++x) {
        // Signals must be added before other methods, to match moc.
        const QMap<QByteArray, Method> &map = (x == 0) ? signals_ : slots_;
        for (QMap<QByteArray, Method>::ConstIterator it = map.constBegin(); it != map.constEnd(); ++it) {
            QByteArray prototype(QMetaObject::normalizedSignature(it.key()));
            QByteArray name = prototype.left(prototype.indexOf('('));
            const auto paramTypeNames = paramList(prototype);
            const QByteArrayList paramNames = it.value().parameters.isEmpty()
                ? QByteArrayList() : it.value().parameters.split(',');
            Q_ASSERT(paramTypeNames.size() == paramNames.size());
            if (!it.value().realPrototype.isEmpty())
                realPrototype_.insert(prototype, it.value().realPrototype);
            int argc = paramTypeNames.size();
            QByteArray tag;
            int_data[offset++] = uint(strings.enter(name));
            int_data[offset++] = uint(argc);
            int_data[offset++] = uint(paramsOffset);
            int_data[offset++] = uint(strings.enter(tag));
            int_data[offset++] = uint(it.value().flags);

            // Parameter types
            for (int i = -1; i < argc; ++i) {
                QByteArray typeName = (i < 0) ? it.value().type : paramTypeNames.at(i);
                int_data[paramsOffset++] = nameToTypeInfo(typeName, strings);
            }
            // Parameter names
            for (int i = 0; i < argc; ++i)
                int_data[paramsOffset++] = uint(strings.enter(paramNames.at(i)));
        }
    }
    Q_ASSERT(offset == header->methodData + header->methodCount * 5);
    Q_ASSERT(paramsOffset = header->propertyData);
    offset += paramsDataSize;
    Q_ASSERT(offset == header->propertyData);

    // each property in form name\0type\0
    for (auto it = properties_.cbegin(), end = properties_.cend(); it != end; ++it) {
        const QByteArray &name = it.key();
        const QByteArray &type = it.value().type;
        Q_ASSERT(!type.isEmpty());
        QByteArray realType(it.value().realType);
        if (!realType.isEmpty() && realType != type)
            realPrototype_.insert(name, realType);
        int_data[offset++] = uint(strings.enter(name));
        int_data[offset++] = nameToTypeInfo(type, strings);
        int_data[offset++] = uint(it.value().flags);
    }
    Q_ASSERT(offset == header->enumeratorData);

    int value_offset = offset + enums_.count() * 5;
    // each enum in form name\0
    for (auto it = enums_.cbegin(), end = enums_.cend(); it != end; ++it) {
        QByteArray name(it.key());
        int count = it.value().count();

        uint nameId = uint(strings.enter(name));
        int_data[offset++] = nameId;
        int_data[offset++] = nameId;
        int_data[offset++] = 0x0; // 0x1 for flag?
        int_data[offset++] = uint(count);
        int_data[offset++] = uint(value_offset);
        value_offset += count * 2;
    }
    Q_ASSERT(offset == header->enumeratorData + enums_.count() * 5);

    // each enum value in form key\0
    for (auto it = enums_.cbegin(), end = enums_.cend(); it != end; ++it) {
        for (const auto &e : it.value()) {
            int_data[offset++] = uint(strings.enter(e.first));
            int_data[offset++] = uint(e.second);
        }
    }
    Q_ASSERT(offset == int_data_size-1);
    int_data[offset] = 0; // eod

    char *string_data = new char[static_cast<size_t>(strings.blobSize())];
    strings.writeBlob(string_data);

    // put the metaobject together
    meta->d.data = int_data;
    meta->d.extradata = nullptr;
    meta->d.stringdata = reinterpret_cast<const QByteArrayData *>(string_data);
    meta->d.static_metacall = nullptr;
    meta->d.relatedMetaObjects = nullptr;
    meta->d.superdata = &QObject::staticMetaObject;
}

int MetaObjectBuilder::aggregateParameterCount(const QMap<QByteArray, MetaObjectBuilder::Method> &map)
{
    int sum = 0;
    QMap<QByteArray, Method>::const_iterator it;
    for (it = map.constBegin(); it != map.constEnd(); ++it)
        sum += paramList(it.key()).size() + 1; // +1 for return type
    return sum;
}

int MetaObjectBuilder::numParameter(const QByteArray &prototype)
{
    if (!memberInfo_.contains(prototype))
        parsePrototype(prototype);
    return memberInfo_.value(prototype).count();
}

QByteArray MetaObjectBuilder::paramType(const QByteArray &prototype, int index, bool *out)
{
    if (!memberInfo_.contains(prototype))
        parsePrototype(prototype);
    if (out)
        *out = false;
    QList<QByteArray> plist = memberInfo_.value(prototype);
    if (index > plist.count() - 1)
        return QByteArray();
    QByteArray param(plist.at(index));
    if (param.isEmpty())
        return QByteArray();
    bool byRef = param.endsWith('&') || param.endsWith("**");
    if (byRef) {
        param.truncate(param.length() - 1);
        if (out)
            *out = true;
    }
    return param;
}


QMetaStringTable::QMetaStringTable(const QByteArray &className)
    : m_index(0)
    , m_className(className)
{
    const int index = enter(m_className);
    Q_ASSERT(index == 0);
    Q_UNUSED(index)
}

// Enters the given value into the string table (if it hasn't already been
// entered). Returns the index of the string.
int QMetaStringTable::enter(const QByteArray &value)
{
    Entries::iterator it = m_entries.find(value);
    if (it != m_entries.end())
        return it.value();
    int pos = m_index;
    m_entries.insert(value, pos);
    ++m_index;
    return pos;
}
int QMetaStringTable::preferredAlignment()
{
    return Q_ALIGNOF(QByteArrayData);
}

// Returns the size (in bytes) required for serializing this string table.
int QMetaStringTable::blobSize() const
{
    int size = m_entries.size() * static_cast<int>(sizeof(QByteArrayData));
    Entries::const_iterator it;
    for (it = m_entries.constBegin(); it != m_entries.constEnd(); ++it)
        size += it.key().size() + 1;
    return size;
}
