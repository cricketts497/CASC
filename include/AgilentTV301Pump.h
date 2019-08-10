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
    void dealWithResponse(QString response);

private:
    QString activeQuery;

};


#endif // AGILENT_TV301_PUMP