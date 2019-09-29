#ifndef QCOMPONENTREGISTRY_HPP
#define QCOMPONENTREGISTRY_HPP

#include "qcomponentregistry.h"

template<typename Func>
QExportBase const * QComponentRegistry::get_export(Func f)
{
    QExportBase const * re = nullptr;
    for (auto m : metas) {
        for (QExportBase * e : m.second.exports) {
            if (f(*e)) {
                if (re == nullptr)
                    re = e;
                else
                    return nullptr;
                break;
            }
        }
    }
    return re;
}

template<typename Func>
std::list<QExportBase const *> QComponentRegistry::get_exports(Func f)
{
    std::list<QExportBase const *> list;
    for (auto m : metas) {
        for (QExportBase * e : m.second.exports) {
            if (f(*e)) {
                list.push_back(e);
                break;
            }
        }
    }
    return list;
}

#endif // QCOMPONENTREGISTRY_HPP
