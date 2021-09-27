#ifndef QTESTCOMPOSITION_H
#define QTESTCOMPOSITION_H

#include "QtComposition_global.h"

#include <qlazy.h>

#include <QObject>
#include <QVector>

class QTCOMPOSITION_EXPORT QTestComposition : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit QTestComposition(QObject *parent = nullptr);

public:
    Q_PROPERTY(QObject* impl MEMBER impl_)
    Q_PROPERTY(QVector<QLazy> impls MEMBER impls_)

signals:

public slots:
    void onComposition();

private:
    QObject * impl_;
    QVector<QLazy> impls_;
};

#endif // QTESTCOMPOSITION_H
