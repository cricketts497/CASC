#include "include/AgilentTV301Pump.h"

AgilentTV301Pump::AgilentTV301Pump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(file_path, file_mutex, deviceName, config, parent),
activeQuery(QString("NONE")),
pumpStatusCode(7),
pumpErrorCode(0)
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
    connect(this, &SerialDevice::serialComFinished, this, &AgilentTV301Pump::pumpCommand);
    
    //setup the query timers
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

    //periodic commands timers
    // temperatureTimer->start();
    // speedStatusTimer->start();
    
    //intial device status: device_name, ....
    deviceStatus = QString("Status_%1_%2_%3").arg(device_name).arg(pumpStatusCode).arg(pumpErrorCode); 
}

void AgilentTV301Pump::pumpCommand()
{
    //no further commands or current command ongoing
    if(serialCommandQueue.isEmpty())
        return;
    
    QString command = serialCommandQueue.dequeue();
	QStringList command_list = command.split("_");
    
    QString toQuery;
    //crc is 0xB4 => ascii 0x42, 0x34
    if(command_list.first() == QString("PUMPSTATUSCODE")){
        toQuery = QString("\x02\x80205\x30\x03\x42\x34");
    //crc is 0xB7 => ascii 0x42, 0x37
    }else if(command_list.first() == QString("PUMPERRORCODE"){
        toQuery = QString("\x02\x80206\x30\x03\x42\x37");
    }else{
        return;
    }
    
    //send the query
    if(activeQuery != QString("NONE")){
        emit device_message(QString("Local NxdsPump: %1: Busy waiting for reply").arg(device_name));
        return;
    }
    if(writeCommand(toQuery, true))
        activeQuery = toQuery;   
}

void AgilentTV301Pump::dealWithResponse(QString response)
{
    //no response was received before the timeout, do nothing
    if(response == noResponseMessage){
        activeQuery = QString("NONE");
        return;
    }
    
    emit device_message(QString("Local AgilentTV301Pump: %1: Response: %2").arg(device_name).arg(response));
    
    if(activeQuery == QString("NONE")){
        return;
    }
    
    //check the length
    if(response.length() < 6 || !response.contains(QChar(0x03))){
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Pump response %2 is invalid for query %3").arg(device_name).arg(response).arg(activeQuery));
        emit device_fail();
        activeQuery = QString("NONE");
        return;
    }
    
    //first two characters of response should be <start transmission>, <address, 0x80>
    if(!response.startsWith(QString("\x02\x80"))){
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Pump response %2 is invalid for query %3").arg(device_name).arg(response).arg(activeQuery));
        emit device_fail();
        activeQuery = QString("NONE");
        return;
    }
    
    int endIndex = response.indexOf(QChar(0x03));
    QString outResponse = response.mid(2, endIndex-2);
    
    //send to functions
    if(activeQuery == QString("\x02\x80205\x30\x03\x42\x34")){
        responsePumpStatuscode(outResponse);
    }else if(activeQuery == QString("\x02\x80206\x30\x03\x42\x37")){
        responsePumpErrorCode(outResponse);
    }else{
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Unknown activeQuery").arg(device_name));
        emit device_fail();
    }
       
    deviceStatus = QString("Status_%1_%2_%3").arg(device_name).arg(pumpStatusCode).arg(pumpErrorCode);
    emit device_message(QString("Local AgilentTV301Pump: %1: device status %2").arg(device_name).arg(deviceStatus));
    
    activeQuery = QString("NONE");
}


//pump responses
/////////////////////////////////////////////////////////////////////////////
void AgilentTV301Pump::responsePumpStatusCode(QString response)
{
    bool conv_ok;
    uint code = response.toUInt(&conv_ok);
    
    //check the code
    if(!conv_ok || code > 6){
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Pump response %2 is invalid for PUMPSTATUSCODE query").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    pumpStatusCode = code;
    
    emit device_message(QString("Local AgilentTV301Pump: %1: Pump status code: %2").arg(device_name).arg(pumpStatusCode));
}

void AgilentTV301Pump::responsePumpErrorCode(QString response)
{
    bool conv_ok;
    uint code = response.toUInt(&conv_ok);
    
    //check the code
    if(!conv_ok || code > 255){
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Pump response %2 is invalid for PUMPSTATUSCODE query").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    pumpErrorCode = code;
    
    if(pumpErrorCode == 0){
        return;
    }
    
    //error codes decoded
    if((pumpErrorCode&0x0001)==0x0001){
        emit device_message(QString("Local AgilentTV301Pump: %1: ERROR: No connection between controller and pump").arg(device_name));
    }
    if((pumpErrorCode&0x0002)==0x0002){
        emit device_message(QString("Local AgilentTV301Pump: %1: ERROR: Pump is over maximum temperature").arg(device_name));
    }
    if((pumpErrorCode&0x0004)==0x0004){
        emit device_message(QString("Local AgilentTV301Pump: %1: ERROR: Controller is over maximum temperature").arg(device_name));
    }
    if((pumpErrorCode&0x0010)==0x0010){
        emit device_message(QString("Local AgilentTV301Pump: %1: ERROR: Power failure").arg(device_name));
    }
    if((pumpErrorCode&0x0020)==0x0020){
        emit device_message(QString("Local AgilentTV301Pump: %1: ERROR: Auxilary failure").arg(device_name));
    }
    if((pumpErrorCode&0x0040)==0x0040){
        emit device_message(QString("Local AgilentTV301Pump: %1: ERROR: Overvoltage").arg(device_name));
    }
    if((pumpErrorCode&0x0080)==0x0080){
        emit device_message(QString("Local AgilentTV301Pump: %1: ERROR: Short circuit").arg(device_name));
    }
    if((pumpErrorCode&0x0100)==0x0100){
        emit device_message(QString("Local AgilentTV301Pump: %1: ERROR: Pumping load is too high").arg(device_name));
    }
}

