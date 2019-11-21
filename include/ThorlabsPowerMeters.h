#ifndef THORLABS_POWER_METERS
#define THORLABS_POWER_METERS

#include "TLPM.h"

#include "include/CascDevice.h"

class ThorlabsPowerMeters : public CascDevice
{
    Q_OBJECT
public:
    ThorlabsPowerMeters(QString deviceName, QObject * parent=nullptr);

private:
    uint nDevices;

};

#endif // THORLABS_POWER_METERS