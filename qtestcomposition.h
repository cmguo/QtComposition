#ifndef QTEST_H
#define QTEST_H

#include "QtComposition_global.h"

#include <qlazy.h>

#include <QObject>

class QTCOMPOSITION_EXPORT QTestComposition : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit QTestComposition(QObject *parent = nullptr);

public:
    Q_PROPERTY(QObject* impl MEMBER impl_)
    Q_PROPERTY(std::vector<QLazy> impls MEMBER impls_)

signals:

public slots:
    void on_composition();

private:
    QObject * impl_;
    std::vector<QLazy> impls_;
};

#endif // QTEST_H
