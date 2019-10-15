#include "qlazy.h"
#include "qcomponentcontainer.h"

QLazy::QLazy()
    : cont_(nullptr)
    , part_(nullptr)
    , share_(false)
    , obj_(nullptr)
{
}

QLazy::QLazy(QComponentContainer * cont, QPart const * part, bool share)
    : cont_(cont)
    , part_(part)
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
