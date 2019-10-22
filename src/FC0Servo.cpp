#include "include/FC0Servo.h"

FC0Servo::FC0Servo(QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(deviceName, config, parent),
inAngle(30),
outAngle(0)
{
    if(device_failed)
        return;
    
    //Settings for serial communication with the pump over usb
    setBaudRate(115200);
    setDataBits(8);
	setParity(0);//No parity
	setStopBits(1);
	setFlowControl(1);//NoFlowControl
    
    //response from device
    connect(this, SIGNAL(newSerialResponse(QByteArray)), this, SLOT(dealWithResponse(QByteArray)));
    
    //communication finished, take next command
    connect(this, &SerialDevice::serialComFinished, this, &FC0Servo::cupCommand);
    
    if(!openSerialPort())
        return;
    
    //initially query the position of the servo
    queueSerialCommand("QUERYANGLE");
}

FC0Servo::~FC0Servo()
{
    emit device_status(QString("Status_%1_off").arg(device_name));
}

void FC0Servo::cupCommand()
{
    //no further commands
    if(serialCommandQueue.isEmpty())
        return;
    
    QString command = serialCommandQueue.dequeue();    
    QString comSend;
    
    if(command == QString("QUERYANGLE")){
        comSend = QString("ANGLE?\r");
    }else{
        QStringList command_list = command.split("_");
        if(command_list.first() != "Command" || command_list.length() != 3){
            return;
        }
        
        //check the epics variable name
        QStringList paramList = command_list.at(1).split(":");
        if(paramList.first() != "CASC" || paramList.length() < 3){
            return;
        }
        if(paramList.at(1) != device_name || paramList.at(2) != QString("StateCommanded")){
            return;
        }
        
        if(command_list.at(2) == QString("In")){
            comSend = QString("ANGLE %1\r").arg(inAngle);
        }else if(command_list.at(2) == QString("Out")){
            comSend = QString("ANGLE %1\r").arg(outAngle);
        }else{
            return;
        }     
    }
    
    //send the query, SerialDevice rejects if busy, expect a response
    writeCommand(comSend.toUtf8(), true);
}

void FC0Servo::dealWithResponse(QByteArray resp)
{
    //no response was received before the timeout, do nothing
    if(resp == noResponseMessage){
        return;
    }
    
    QString response = QString::fromUtf8(resp);
    
    QStringList response_list = response.split("\r\n");
    
    bool conv_ok;
    for(int i=0; i<response_list.length(); i++){
        //two parts of a response: the window and the value
        QStringList response_parts = response_list.at(i).split(":");
        
        //partial response
        if(response_parts.length()!=2 || response_parts.first() != QString("ANGLE")){
            continue;
        }
        
        uint angle = response_parts.at(1).toUInt(&conv_ok);
        
        if(angle == inAngle){
            setStatus(QString("Status_%1_In").arg(device_name));
        }else if(angle == outAngle){
            setStatus(QString("Status_%1_Out").arg(device_name));
        }else{
            setStatus(QString("Status_%1_InvalidAngle").arg(device_name));
        }            
    }
    
    //free up the serial device
    fullResponseReceived();
}