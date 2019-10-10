#ifndef AGILENT_TV301_PUMP
#define AGILENT_TV301_PUMP

#include "include/SerialDevice.h"

class AgilentTV301Pump : public SerialDevice
{
    Q_OBJECT
public:
    AgilentTV301Pump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent=nullptr);
    
private slots:
    void queryStatus();
    void queryErrorCode();
    void queryTemperature();
    void queryDrive();

    void pumpCommand();
    void dealWithResponse(QByteArray response);

private:    
    void responsePumpStatusCode(QString response);
    void responsePumpErrorCode(QString response);
    void responsePumpTemperature(QString response);
    void responsePumpDrive(QString response);
    
    //query timers
    QTimer * statusTimer;
    const int statusTimeout;
   
    QTimer * errorCodeTimer;
    const int errorCodeTimeout;
    
    QTimer * temperatureTimer;
    const int temperatureTimeout;
    
    QTimer * driveTimer;
    const int driveTimeout;
    
    QString pumpStatus;
    QString pumpError;
    uint pumpTemperature;
    uint pumpDrive;
    
    QString getHexValues(QByteArray response);
    
    QByteArray fullResponse;
};


#endif // AGILENT_TV301_PUMP