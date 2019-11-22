#include "include/ThorlabsPowerMeters.h"

ThorlabsPowerMeters::ThorlabsPowerMeters(QString deviceName, QObject * parent) :
CascDevice(deviceName, parent),
nDevices(0)
{
    if(device_failed){
        return;
    }    
    
    //find the connected power meters
    ViStatus status = TLPM_findRsrc(0,&nDevices);
    if(!queryStatus(status)){
        storeMessage(QString("ThorlabsPowerMeters: %1: Found %2 device(s)").arg(device_name).arg(nDevices), false);
    }
    
    for(ViUInt32 i=0; i<nDevices; i++){
        ViChar resourceName[256];
        status = TLPM_getRsrcName(0,i,resourceName);
        queryStatus(status);
        
        ViChar modelName[256];
        ViChar serialNumber[256];
        ViBoolean deviceAvailable = false;
        status = TLPM_getRsrcInfo(0,i,modelName,serialNumber,VI_NULL,&deviceAvailable);
        
        if(deviceAvailable && !queryStatus(status)){
            //initialises the device, resetting it and returning an instrument handle for future communication
            sessions.append(ViSession());
            status = TLPM_init(resourceName, false, true, &sessions.last());
            
            if(queryStatus(status)){
                continue;
            }
            
            ViChar sensorName[256];
            ViChar sensorSerialNumber[256];
            ViChar sensorCalibrationMessage[256];
            ViInt16 sensorType;
            ViInt16 sensorSubType;
            ViInt16 sensorFlags;
            status = TLPM_getSensorInfo(sessions.last(), sensorName, sensorSerialNumber, sensorCalibrationMessage, &sensorType, &sensorSubType, &sensorFlags);
            
            if(queryStatus(status)){
                continue;
            }
            
            storeMessage(QString("ThorlabsPowerMeters: %1: Meter:[modelName = %2, serialNumber = %3], Sensor:[name=%4, serialNumber=%5, calibrationMessage=%6]").arg(device_name).arg(modelName).arg(serialNumber).arg(sensorName).arg(sensorSerialNumber).arg(sensorCalibrationMessage), false);
        }
    }
    
    setStatus("1_1");

}

void ThorlabsPowerMeters::stop_device()
{
    for(int i=0;i<sessions.length();i++){
        ViStatus status = TLPM_close(sessions.at(i));
    }
    emit stopped();
}

bool ThorlabsPowerMeters::queryStatus(ViStatus status)
{
    bool error = (status != 0);
    
    if(error){
        ViChar message[512];
        TLPM_errorMessage(VI_NULL, status, message);
    
        emit device_message(QString("ThorlabsPowerMeters: %1: DEVICE ERROR: %2").arg(device_name).arg(message));    
        storeMessage(QString("ThorlabsPowerMeters: %1: DEVICE ERROR: %2").arg(device_name).arg(message), true);  
        emit device_fail();
    }
    
    return error;
}