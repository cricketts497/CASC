#ifndef FC0_SERVO
#define FC0_SERVO

#include "include/SerialDevice.h"

class FC0Servo : public SerialDevice
{
    Q_OBJECT
public:
    FC0Servo(QString deviceName, CascConfig * config, QObject * parent = nullptr);
    ~FC0Servo();
    
private slots:
    void cupCommand();
    void dealWithResponse(QByteArray resp);
    
private:
    const uint inAngle;
    const uint outAngle;
    
};


#endif // FC0_SERVO
