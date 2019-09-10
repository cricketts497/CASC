#ifndef NXDS_PUMP_H
#define NXDS_PUMP_H

#include "include/SerialDevice.h"

class NxdsPump : public SerialDevice
{
    Q_OBJECT
    
public:
    NxdsPump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent=nullptr);

private slots:
    void queryTemperature();
    void querySpeedStatus();

    void pumpCommand();
    
    void dealWithResponse(QByteArray response);

private:    
    //responses
    void responsePumpType(QString response);
    void responsePumpServiceStatus(QString response);
    void responsePumpTemperature(QString response);
    void responsePumpSpeedStatus(QString response);
    void responsePumpResetTipSeal(QString response);
    
    //query timers
    QTimer * temperatureTimer;
    const int temperatureTimeout;
    
    QTimer * speedStatusTimer;
    const int speedStatusTimeout;
    
    QString pumpStatus;
    QString pumpControllerTemperature;
    QString pumpServiceStatus;
};

#endif // NXDS_PUMP_H