#include "qlazy.h"
#include "qcomponentcontainer.h"

QLazy::QLazy()
    : cont_(nullptr)
    , meta_(nullptr)
    , share_(false)
    , obj_(nullptr)
{
}

QLazy::QLazy(QComponentContainer * cont, QMetaObject const * meta, bool share)
    : cont_(cont)
    , meta_(meta)
    , share_(share)
    , obj_(nullptr)
{
}

QObject * QLazy::get_()
{
    if (obj_ == nullptr && cont_ != nullptr)
        obj_ = cont_->get_export_value(*this);
    return obj_;
}

