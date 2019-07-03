#ifndef NXDS_PUMP_H
#define NXDS_PUMP_H

#include "include/SerialDevice.h"

class NxdsPump : public SerialDevice
{
    Q_OBJECT
    
public:
    NxdsPump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent=nullptr);

private slots:
    void pumpCommand();
    
    void dealWithResponse();

private:
    QString activeQuery;
    
    //responses
    void responsePumpType(QString response);

};

#endif // NXDS_PUMP_H