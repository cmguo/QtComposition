#ifndef QLAZY_H
#define QLAZY_H

#include "QtComposition_global.h"

#include <QObject>

class QComponentContainer;

class QTCOMPOSITION_EXPORT QLazy
{
public:
    QLazy();

protected:
    QLazy(QComponentContainer * cont, QMetaObject const * meta, bool share);

public:
    template<typename T>
    T * get()
    {
        return qobject_cast<T *>(get_());
    }

    friend
    bool operator==(QLazy const & l, QLazy const & r)
    {
        return l.cont_ == r.cont_ && l.meta_ == r.meta_ && l.share_ == r.share_;
    }

private:
    QObject * get_();

private:
    friend class QComponentContainer;
    QComponentContainer * cont_;
    QMetaObject const * meta_;
    bool share_;
    QObject * obj_;
};

Q_DECLARE_METATYPE(QLazy)

#endif // QLAZY_H
