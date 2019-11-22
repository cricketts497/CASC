#ifndef THORLABS_POWER_METERS
#define THORLABS_POWER_METERS

#include "TLPM.h"

#include "include/CascDevice.h"

class ThorlabsPowerMeters : public CascDevice
{
    Q_OBJECT
public:
    ThorlabsPowerMeters(QString deviceName, QObject * parent=nullptr);

public slots:
    void stop_device();

private:
    bool queryStatus(ViStatus status);

    ViUInt32 nDevices;
    
    QList<ViSession> sessions;
    

};

#endif // THORLABS_POWER_METERS