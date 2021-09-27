#ifndef QCOMPONENTFACTORYINTERFACE_H
#define QCOMPONENTFACTORYINTERFACE_H

#include "QtComposition_global.h"

#include <QObject>

class QComponentFactoryInterface
{
public:
};

#define ComponentFactory_iid \
    "org.qt-project.Qt.ComponentFactoryInterface/1.0"

Q_DECLARE_INTERFACE(QComponentFactoryInterface, ComponentFactory_iid)

#endif // QCOMPONENTFACTORYINTERFACE_H
