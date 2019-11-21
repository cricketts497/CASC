#include "include/ThorlabsPowerMeters.h"

ThorlabsPowerMeters::ThorlabsPowerMeters(QString deviceName, QObject * parent) :
CascDevice(deviceName, parent)
{
    if(device_failed){
        return;
    }    
    
    //find the connected power meters
    ViStatus status;
    
    status = TLPM_findRsrc(0,nDevices));
    queryStatus(status);
    
    emit device_message(QString("ThorlabsPowerMeters: %1: Found %2 devices").arg(device_name).arg(nDevices));
}

void ThorlabsPowerMeters::queryStatus(ViStatus status)
{
    if(status != 0){
        ViChar * message[512];
        ViStatus error;
        error = TLPM_errorMessage(VI_NULL, status, message);
    
        QString error_message(message);
        emit device_message(QString("ThorlabsPowerMeters: %1: Device error: %2").arg(device_name).arg(error_message));    
    }
}