#ifndef QIMPORT_H
#define QIMPORT_H

#include "QtComposition_global.h"
#include "qpart.h"
#include "qlazy.h"

#include <vector>

class QExportBase;
class QComponentRegistry;

class QTCOMPOSITION_EXPORT QImportBase : public QPart
{
public:
    QImportBase(QMetaObject const * meta, char const * prop);

    QImportBase(QMetaObject const * meta, char const * prop, QMetaObject const * type);

    QImportBase(QMetaObject const * meta, char const * prop, Share share, bool lazy, QMetaObject const * type, char const * name);

public:
    void set_many() { count_ = Import::many; }

    void set_optional() { count_ = Import::optional; }

    bool valid() const;

    void compose(QObject * obj, QObject * target) const;

    void compose(QObject * obj, std::vector<QObject *> const & targets) const;

    void compose(QObject * obj, QLazy target) const;

    void compose(QObject * obj, std::vector<QLazy> const & targets) const;

private:
    friend class QComponentRegistry;
    char const * prop_;
    Import count_;
    bool lazy_;
    std::vector<QExportBase const *> exports;
};

template <typename T, typename U>
class QImport : QImportBase
{
public:
    QImport(char const * prop, Share share = Share::any, bool lazy = false)
        : QImportBase(
              &T::staticMetaObject,
              prop, share, lazy,
              &U::staticMetaObject,
              nullptr)
    {
    }

    QImport(char const * prop, char const * name, Share share = Share::any, bool lazy = false)
        : QImportBase(
              &T::staticMetaObject,
              prop, share, lazy,
              &U::staticMetaObject,
              name)
    {
    }

};

template <typename T, typename U>
class QImportMany : QImportBase
{
public:
    QImportMany(char const * prop, Share share = Share::any, bool lazy = false)
        : QImportBase(
              &T::staticMetaObject,
              prop, share, lazy,
              &U::staticMetaObject,
              nullptr)
    {
        set_many();
    }

    QImportMany(char const * prop, char const * name, Share share = Share::any, bool lazy = false)
        : QImportBase(
              &T::staticMetaObject,
              prop, share, lazy,
              &U::staticMetaObject,
              name)
    {
        set_many();
    }

};

template <typename T, typename U>
class QImportOptional : QImportBase
{
public:
    QImportOptional(char const * prop, Share share = Share::any, bool lazy = false)
        : QImportBase(
              &T::staticMetaObject,
              prop, share, lazy,
              &U::staticMetaObject,
              nullptr)
    {
        set_optional();
    }

    QImportOptional(char const * prop, char const * name, Share share = Share::any, bool lazy = false)
        : QImportBase(
              &T::staticMetaObject,
              prop, share, lazy,
              &U::staticMetaObject,
              name)
    {
        set_optional();
    }

};

#endif // QIMPORT_H
