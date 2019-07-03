#include "include/NxdsPump.h"

NxdsPump::NxdsPump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(file_path, file_mutex, deviceName, config, parent),
activeQuery(QString("NONE"))
{
    if(device_failed)
        return;
    
    //Settings for serial communication with the pump over usb
    setBaudRate(9600);
    setDataBits(8);
	setParity(0);//No parity
	setStopBits(1);
	setFlowControl(1);//NoFlowControl
    
    
    
    
    //response from device
    connect(this, SIGNAL(newSerialResponse(QString)), this, SLOT(dealWithResponse(QString)));
    
    //communication finished, take next command
    connect(this, &SerialDevice::serialComFinished, this, &NxdsPump::pumpCommand);
    
    if(!openSerialPort())
        return;
    
}


void NxdsPump::pumpCommand()
{
    //no further commands or current command ongoing
    if(serialCommandQueue.isEmpty())
        return;
    
    QString command = serialCommandQueue.dequeue();
	QStringList command_list = command.split("_");
    
    
    
    QString toQuery;
    if(command_list.first() == QString("PUMPTYPE")){
        toQuery = QString("?S801\r");
    }
    
    
    //send the query
    if(activeQuery != QString("NONE")){
        emit device_message(QString("Local NxdsPump: %1: Busy waiting for reply").arg(device_name));
        return;
    }
    if(writeCommand(toQuery, true))
        activeQuery = toQuery;
}

void NxdsPump::dealWithResponse(QString response)
{
    if(response == noResponseMessage){
        activeQuery = QString("NONE");
        return;
    }
    
    emit device_message(QString("Local NxdsPump: %1: Response: %2").arg(device_name).arg(response));
    
    if(activeQuery == QString("NONE")){
        return;
    }
    
    //numbers in the query should be same as response
    QStringRef subResp(&response, 2,3);
    QStringRef subQuery(&activeQuery, 2,3);
    if(subResp != subQuery){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Pump response %2 is invalid for query %3").arg(device_name).arg(response).arg(activeQuery));
        emit device_fail();
    }
    
    //all characters from the 6th slot and after
    QString outResponse = response.mid(6);
    
    //send to functions
    if(activeQuery == QString("?S801\r")){
        responsePumpType(outResponse);
    }else{
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Unknown activeQuery").arg(device_name));
        emit device_fail();
    }
       
    activeQuery = QString("NONE");
}


//pump responses
////////////////////////////////////////////////////////////////////////////////////////////////
void NxdsPump::responsePumpType(QString response)
{
    
}


