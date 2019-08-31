#ifndef AGILENT_TV301_PUMP
#define AGILENT_TV301_PUMP

#include "include/SerialDevice.h"

class AgilentTV301Pump : public SerialDevice
{
    Q_OBJECT
public:
    AgilentTV301Pump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent=nullptr);
    
private slots:
    void pumpCommand();
    void dealWithResponse(QByteArray response);

private:
    QString activeQuery;
    
    void responsePumpStatusCode(QString response);
    void responsePumpErrorCode(QString response);
    void responsePumpTemperature(QString response);
    void responsePumpDrive(QString response);
    
    uint pumpStatusCode;
    uint pumpErrorCode;
    uint pumpTemperature;
    uint pumpDrive;
};


#endif // AGILENT_TV301_PUMP