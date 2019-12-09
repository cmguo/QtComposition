#include "qcomponentregistry.h"
#include "qcomponentcontainer.h"

struct QComponentRegistry::Meta
{
    Meta() : invalid(false){}

    bool invalid;
    QVector<QExportBase *> exports;
    QVector<QImportBase *> imports;
};

std::map<QMetaObject const *, QComponentRegistry::Meta> QComponentRegistry::metas_;

void QComponentRegistry::add_export(QExportBase * e)
{
    get_meta(e->meta_).exports.push_back(e);
}

void QComponentRegistry::add_import(QImportBase * i)
{
    get_meta(i->meta_).imports.push_back(i);
}

void QComponentRegistry::composition()
{
    static bool composed = false;
    if (composed == true)
        return;
    composed = true;
    QVector<QMetaObject const *> invalids;
    size_t count = 0;
    for (auto& m : metas_) {
        for (QImportBase * i : m.second.imports) {
            i->exports = get_exports(*i);
            if (!i->valid()) {
                m.second.invalid = true;
                invalids.push_back(m.first);
                break;
            }
        }
    }
    while (count < invalids.size()) {
        QMetaObject const * meta = invalids[count++];
        for (auto &m : metas_) {
            if (m.second.invalid)
                continue;
            if (m.first->inherits(meta)) {
                m.second.invalid = true;
                invalids.push_back(m.first);
                continue;
            }
            for (QImportBase * i : m.second.imports) {
                i->exports.erase(
                            std::remove_if(i->exports.begin(), i->exports.end(),
                                           [meta](auto e) { return e->meta_ == meta;}),
                            i->exports.end());
                if (!i->valid()) {
                    m.second.invalid = true;
                    invalids.push_back(m.first);
                    break;
                }
            }
        }
    }
}

void QComponentRegistry::compose(
        QComponentContainer * cont, QMetaObject const & type, QObject * obj)
{
    if (&type == &QObject::staticMetaObject)
        return;
    auto iter = metas_.find(&type);
    if (iter == metas_.end()) {
        compose(cont, *type.superClass(), obj);
        return;
    }
    Meta const & meta = iter->second;
    for (auto i : meta.imports) {
        if (i->count_ == QImportBase::many)
            if (i->lazy_)
                i->compose(obj, cont->get_exports(*i));
            else
                i->compose(obj, cont->get_export_values(*i));
        else
            if (i->lazy_)
                i->compose(obj, cont->get_export(*i));
            else
                i->compose(obj, cont->get_export_value(*i));
    }
    compose(cont, *type.superClass(), obj);
}

QComponentRegistry::Meta & QComponentRegistry::get_meta(QMetaObject const * meta)
{
    auto iter = metas_.find(meta);
    if (iter == metas_.end()) {
        iter = metas_.insert(std::make_pair(meta, Meta())).first;
    }
    return iter->second;
}

QVector<QExportBase const *> QComponentRegistry::get_exports(QPart const & i)
{
    QVector<QExportBase const *> list;
    for (auto m : metas_) {
        for (QExportBase * e : m.second.exports) {
            if (e->match(i) && !metas_[e->meta_].invalid) {
                list.push_back(e);
                break;
            }
        }
    }
    return list;
}
