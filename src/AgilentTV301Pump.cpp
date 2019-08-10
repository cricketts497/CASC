#include "include/AgilentTV301Pump.h"

AgilentTV301Pump::AgilentTV301Pump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(file_path, file_mutex, deviceName, config, parent)
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
    deviceStatus = QString("Status_%1_0_0_0").arg(device_name); 
}

void AgilentTV301Pump::pumpCommand()
{
    //no further commands or current command ongoing
    if(serialCommandQueue.isEmpty())
        return;
    
    QString command = serialCommandQueue.dequeue();
	QStringList command_list = command.split("_");
    
    QString toQuery;
    //////
    //if statements for commands
    //////
    if(command_list.first() == QString("PUMPSTATUS")){
        toQuery = QString("\x02\x80205\x30\x03");
    
    // //prepend the start transmission and address, and append the end transmission
    // quint8 command_address_int = 0x80;
    // char command_start_trans = '\u0002';
    // char command_end_trans = '\u0003';

    // toQuery.append(command_end_trans);
    
    // //calculate the CRC by xor-ing each character with the next in turn (without the start trans) and append it to the query
    // QByteArray command_array = toQuery.toUtf8();
    // int crc = command_address;
    // for(int i=0;i<command_array.size();i++){
        // crc = crc ^ command_array.at(i);
    // }
    
    // //left character of final crc
    // int left = crc - crc % 0x10;
    
    // //right character of final crc in hex
    // int right = crc % 0x10;
    
    
    // toQuery.prepend(command_address);
    // toQuery.prepend(command_start_trans);
    
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
    
    
}

