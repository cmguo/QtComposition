#ifndef QLAZY_HPP
#define QLAZY_HPP

#include "qlazy.h"
#include "qcomponentcontainer.h"

template<typename T, typename ...Args>
inline T * QLazy::get(Args && ...args)
{
    if (obj_ == nullptr && cont_ != nullptr)
        obj_ = cont_->getExportValue(*this, std::move(args)...);
    return qobject_cast<T *>(obj_);
}

template<typename T, typename ...Args>
inline T * QLazy::create(Args && ...args)
{
    QObject * obj = cont_->getExportValue(*this, std::move(args)...);
    return qobject_cast<T *>(obj);
}

#endif // QLAZY_HPP
