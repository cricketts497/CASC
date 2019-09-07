#include "include/LaseLock.h"

LaseLock::LaseLock(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(file_path, file_mutex, deviceName, config, parent),
activeQuery(QString("NONE")),
regALocked(2),
regBLocked(2),
regASearch(2),
regBSearch(2),
regARelock(2),
regBRelock(2),
regAHold(2),
regBHold(2)
{
    if(device_failed)
        return;
    
    //Settings for serial communication with the pump over usb
    setBaudRate(57600);
    setDataBits(8);
	setParity(0);//No parity
	setStopBits(1);
	setFlowControl(1);//NoFlowControl
    
    //response from device
    connect(this, SIGNAL(newSerialResponse(QByteArray)), this, SLOT(dealWithResponse(QByteArray)));
    
    //communication finished, take next command
    connect(this, &SerialDevice::serialComFinished, this, &LaseLock::lockCommand);
    
    // //setup the query timers
    // temperatureTimer->setInterval(temperatureTimeout);
    // connect(temperatureTimer, SIGNAL(timeout()), this, SLOT(queryTemperature()));
    // connect(this, SIGNAL(device_fail()), temperatureTimer, SLOT(stop()));
    
    // speedStatusTimer->setInterval(speedStatusTimeout);
    // connect(speedStatusTimer, SIGNAL(timeout()), this, SLOT(querySpeedStatus()));
    // connect(this, SIGNAL(device_fail()), speedStatusTimer, SLOT(stop()));
    
    if(!openSerialPort())
        return;
    
    //initial commands
    // queueSerialCommand(QString("PUMPTYPE"));
    // queueSerialCommand(QString("PUMPSERVICESTATUS"));

    //start the query timers
    // temperatureTimer->start();
    // speedStatusTimer->start();
    
    //set the status
    //device_name, regALocked
    deviceStatus = QString("Status_%1_%2_%3_%4_%5_%6_%7_%8_%9").arg(device_name).arg(regALocked).arg(regBLocked).arg(regASearch).arg(regBSearch).arg(regARelock).arg(regBRelock).arg(regAHold).arg(regBHold);
}

void LaseLock::lockCommand()
{
    //no further commands
    if(serialCommandQueue.isEmpty())
        return;
    
    QString command = serialCommandQueue.dequeue();
	QStringList command_list = command.split("_");
    
    QString toQuery;
    if(command_list.first() == QString("REGALOCKED")){
        toQuery = QString("LockedA=\r");
    }else if(command_list.first() == QString("REGBLOCKED")){
        toQuery = QString("LockedB=\r");
    }else if(command_list.first() == QString("REGASEARCH")){
        toQuery = QString("SearchA=\r");
    }else if(command_list.first() == QString("REGBSEARCH")){
        toQuery = QString("SearchB=\r");
    }else if(command_list.first() == QString("REGARELOCK")){
        toQuery = QString("RelockA=\r");
    }else if(command_list.first() == QString("REGBRELOCK")){
        toQuery = QString("RelockB=\r");
    }else if(command_list.first() == QString("REGAHOLD")){
        toQuery = QString("HoldA=\r");
    }else if(command_list.first() == QString("REGBHOLD")){
        toQuery = QString("HoldB=\r");
    }else{
        return;
    }
    
    //send the query
    if(activeQuery != QString("NONE")){
        emit device_message(QString("Local LaseLock: %1: Busy waiting for reply").arg(device_name));
        return;
    }
    if(writeCommand(toQuery.toUtf8(), true))
        activeQuery = command_list.first();
}

void LaseLock::dealWithResponse(QByteArray resp)
{
    //no response was received before the timeout, do nothing
    if(resp == noResponseMessage){
        activeQuery = QString("NONE");
        return;
    }
    
    QString response = QString::fromUtf8(resp);
    
    emit device_message(QString("Local LaseLock: %1: Response: %2").arg(device_name).arg(response));
    
    //not expecting a response
    if(activeQuery == QString("NONE")){
        return;
    }
    
    //////////////////////////////////
    //check the response
    //////////////////////////////////
    
    if(activeQuery == QString("REGALOCKED")){
        checkBoolResponse(response, &regALocked);
    }else if(activeQuery == QString("REGBLOCKED")){
        checkBoolResponse(response, &regBLocked);
    }else if(activeQuery == QString("REGASEARCH")){
        checkBoolResponse(response, &regASearch);
    }else if(activeQuery == QString("REGBSEARCH")){
        checkBoolResponse(response, &regBSearch);
    }else if(activeQuery == QString("REGARELOCK")){
        checkBoolResponse(response, &regARelock);
    }else if(activeQuery == QString("REGBRELOCK")){
        checkBoolResponse(response, &regBRelock);
    }else if(activeQuery == QString("REGAHOLD")){
        checkBoolResponse(response, &regAHold);
    }else if(activeQuery == QString("REGBHOLD")){
        checkBoolResponse(response, &regBHold);
    }else{
        emit device_message(QString("LOCAL LASELOCK ERROR: %1: Unknown activeQuery").arg(device_name));
        emit device_fail();
    }
    
    //set the status
    deviceStatus = QString("Status_%1_%2_%3_%4_%5_%6_%7_%8_%9").arg(device_name).arg(regALocked).arg(regBLocked).arg(regASearch).arg(regBSearch).arg(regARelock).arg(regBRelock).arg(regAHold).arg(regBHold);
    
    activeQuery = QString("NONE");
    //free up the serial device
    fullResponseReceived();     
}

//LaseLock box responses
////////////////////////////////////////////////////////////////////////////////////////////////////////
void LaseLock::checkBoolResponse(QString response, uint * flag)
{
    bool conv_ok;
    uint bool_val = response.toUInt(&conv_ok);

    if(!conv_ok || bool_val > 1){
        emit device_message(QString("LOCAL LASELOCK ERROR: %1: Response %2 is invalid for boolean query").arg(device_name).arg(response));
        emit device_fail();
    }else{
        *flag = bool_val;
    }
}




