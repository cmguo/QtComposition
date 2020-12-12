#include "qimport.h"

#include <QObject>
#include <QMetaProperty>
#include <QVariant>

#include <stdexcept>

QImportBase::QImportBase(QMetaObject const * meta, char const * prop)
    : QPart(meta, false)
    , prop_(prop)
    , count_(Import::exactly)
    , lazy_(false)
    , typeRegister_(nullptr)
{
}

QImportBase::QImportBase(const QVariantMap &desc)
    : QPart(desc, false)
    , prop_(nullptr)
    , count_(Import::exactly)
    , lazy_(false)
    , typeRegister_(nullptr)
{
    static QStringList countNames = {"optional", "exactly", "many"};
    QByteArray prop = desc.value("prop").toByteArray();
    if (prop.isEmpty())
        throw std::runtime_error("import prop error");
    prop_ = registerString(prop);
    if (desc.contains("count")) {
        int count = countNames.indexOf(desc.value("count").toString());
        if (count < 0)
            throw std::runtime_error("import count error");
        count_ = static_cast<Import>(count);
    }
    lazy_ = desc.value("lazy").toBool();
}

bool QImportBase::checkType()
{
    int i = meta_->indexOfProperty(prop_);
    if (i < 0)
        return type_ != AUTO_META;
    QMetaProperty p = meta_->property(i);
    QByteArray tm = p.typeName();
    int t = p.userType();
    if (tm.endsWith('>') && (tm.startsWith("QVector<")
            || tm.startsWith("QList<")
            || tm.startsWith("std::vector<")
            || tm.startsWith("std::list<"))) {
        if (count_ != many)
            return false;
        tm = tm.mid(tm.indexOf('<') + 1);
        tm = tm.left(tm.length() - 1);
        t = QMetaType::type(tm);
    }
    if (t == 0)
        return true; // we can't check, maybe explicit register
    if (t == qMetaTypeId<QLazy>()) {
        lazy_ = true;
        typeRegister_ = nullptr;
        return true;
    }
    QMetaType mt(t);
    if (!mt.isValid())
        return false;
    QMetaObject const * pm = mt.metaObject();
    if (pm == nullptr || (pm != &QObject::staticMetaObject
                          && !pm->inherits(&QObject::staticMetaObject)))
        return false;
    if (type_ == AUTO_META)
        type_ = pm;
    else if (type_ && !type_->inherits(pm))
        return false;
    return true;
}

bool QImportBase::valid() const
{
    if (count_ == Import::exactly)
        return exports.size() == 1;
    else if (count_ == Import::optional)
        return exports.size() <= 1;
    else
        return true;
}

void QImportBase::compose(QObject * obj, QObject * target) const
{
    if (typeRegister_)
        typeRegister_();
    obj->setProperty(prop_, QVariant::fromValue(target));
}

void QImportBase::compose(QObject * obj, QVector<QObject *> const & targets) const
{
    if (typeRegister_)
        typeRegister_();
    obj->setProperty(prop_, QVariant::fromValue(targets));
}

void QImportBase::compose(QObject * obj, QLazy target) const
{
    if (typeRegister_)
        typeRegister_();
    obj->setProperty(prop_, QVariant::fromValue(target));
}

void QImportBase::compose(QObject * obj, QVector<QLazy> const & targets) const
{
    if (typeRegister_)
        typeRegister_();
    obj->setProperty(prop_, QVariant::fromValue(targets));
}
