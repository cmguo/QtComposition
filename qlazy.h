#ifndef QLAZY_H
#define QLAZY_H

#include "QtComposition_global.h"

#include <QObject>

class QComponentContainer;
class QPart;

class QTCOMPOSITION_EXPORT QLazy
{
public:
    QLazy();

protected:
    QLazy(QComponentContainer * cont, QPart const * part, bool share);

public:
    template<typename T>
    T * get()
    {
        return qobject_cast<T *>(get_());
    }

    template<typename T, typename ...Args>
    T * get(Args && ...args);

    template<typename T, typename ...Args>
    T * create(Args && ...args);

    friend
    bool operator==(QLazy const & l, QLazy const & r)
    {
        return l.cont_ == r.cont_ && l.part_ == r.part_ && l.share_ == r.share_;
    }

    QPart const * part() const
    {
        return part_;
    }

private:
    QObject * get_();

private:
    friend class QComponentContainer;
    QComponentContainer * cont_;
    QPart const * part_;
    bool share_;
    QObject * obj_;
};

Q_DECLARE_METATYPE(QLazy)

#endif // QLAZY_H
