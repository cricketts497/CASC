#include "include/AgilentTV301Pump.h"

AgilentTV301Pump::AgilentTV301Pump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(file_path, file_mutex, deviceName, config, parent),
activeQuery(QString("NONE")),
pumpStatusCode(7),
pumpErrorCode(0),
pumpTemperature(0),
pumpDrive(0)
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
    connect(this, SIGNAL(newSerialResponse(QByteArray)), this, SLOT(dealWithResponse(QByteArray)));
    
    //communication finished, take next command
    connect(this, &SerialDevice::serialComFinished, this, &AgilentTV301Pump::pumpCommand);
    
    //setup the query timers
    // temperatureTimer->setInterval(temperatureTimeout);
    // connect(temperatureTimer, SIGNAL(timeout()), this, SLOT(queryTemperature()));
    // connect(this, SIGNAL(device_fail()), temperatureTimer, SLOT(stop()));
    
    // speedStatusTimer->setInterval(speedStatusTimeout);
    // connect(speedStatusTimer, SIGNAL(timeout()), this, SLOT(querySpeedStatus()));
    // connect(this, SIGNAL(device_fail()), speedStatusTimer, SLOT(stop()));
    
    
    ///////////////////////////////////////////////////////////////////////////////////////////////
    if(!openSerialPort())
        return;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    
    //initial commands
    // queueSerialCommand(QString("PUMPSTATUSCODE"));
    // queueSerialCommand(QString("PUMPERRORCODE"));

    //periodic commands timers
    // temperatureTimer->start();
    // speedStatusTimer->start();
    
    //intial device status: device_name, statusCode, errorCode, temperature
    deviceStatus = QString("Status_%1_%2_%3_%4_%5").arg(device_name).arg(pumpStatusCode).arg(pumpErrorCode).arg(pumpTemperature).arg(pumpDrive); 
}

void AgilentTV301Pump::pumpCommand()
{
    //no further commands or current command ongoing
    if(serialCommandQueue.isEmpty())
        return;
    
    QString command = serialCommandQueue.dequeue();
	QStringList command_list = command.split("_");
    
    QByteArray toQuery;
    toQuery.resize(9);
    //crc is 0xB4 => ascii 0x42, 0x34
    if(command_list.first() == QString("PUMPSTATUSCODE")){
        toQuery[0] = 0x02;
        toQuery[1] = 0x80;
        toQuery[2] = 0x32;
        toQuery[3] = 0x30;
        toQuery[4] = 0x35;
        toQuery[5] = 0x30;
        toQuery[6] = 0x03;
        toQuery[7] = 0x42;
        toQuery[8] = 0x34;       
    //crc is 0xB7 => ascii 0x42, 0x37
    }else if(command_list.first() == QString("PUMPERRORCODE")){
        toQuery[0] = 0x02;
        toQuery[1] = 0x80;
        toQuery[2] = 0x32;
        toQuery[3] = 0x30;
        toQuery[4] = 0x36;
        toQuery[5] = 0x30;
        toQuery[6] = 0x03;
        toQuery[7] = 0x42;
        toQuery[8] = 0x37;
    //crc is 0x85 => ascii 0x38, 0x35
    }else if(command_list.first() == QString("PUMPTEMPERATURE")){
        toQuery[0] = 0x02;
        toQuery[1] = 0x80;
        toQuery[2] = 0x32;
        toQuery[3] = 0x30;
        toQuery[4] = 0x34;
        toQuery[5] = 0x30;
        toQuery[6] = 0x03;
        toQuery[7] = 0x38;
        toQuery[8] = 0x35;
    //crc is 0x82 => ascii 0x38, 0x32
    }else if(command_list.first() == QString("PUMPDRIVE")){
        toQuery[0] = 0x02;
        toQuery[1] = 0x80;
        toQuery[2] = 0x32;
        toQuery[3] = 0x30;
        toQuery[4] = 0x33;
        toQuery[5] = 0x30;
        toQuery[6] = 0x03;
        toQuery[7] = 0x38;
        toQuery[8] = 0x32;
    }else{
        return;
    }
    
    //send the query
    if(activeQuery != QString("NONE")){
        emit device_message(QString("Local AgilentTV301Pump: %1: Busy waiting for reply").arg(device_name));
        return;
    }
    
    QString hexValues = "";
    for(int i=0; i<toQuery.length(); i++){
        ushort charac = ushort(uchar(toQuery.at(i)));
        hexValues.append(QString("0x%1,").arg(charac,0,16));
    }
    storeMessage(QString("Local AgilentTV301Pump: %1: Sending query %2").arg(device_name).arg(hexValues), false);
    
    if(writeCommand(toQuery, true))
        activeQuery = command_list.first();
}

void AgilentTV301Pump::dealWithResponse(QByteArray response)
{
    //no response was received before the timeout, do nothing
    if(response == noResponseMessage){
        activeQuery = QString("NONE");
        return;
    }
    
    emit device_message(QString("Local AgilentTV301Pump: %1: Response: %2").arg(device_name).arg(QString::fromUtf8(response)));
    
    if(activeQuery == QString("NONE")){
        return;
    }
    
    //check the length
    if(response.length() < 6 || !response.contains(char(0x03)) || !response.startsWith(char(0x02))){
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Pump response %2 is invalid for query %3").arg(device_name).arg(QString::fromUtf8(response)).arg(activeQuery));
        emit device_fail();
        activeQuery = QString("NONE");
        return;
    }
    
    int endIndex = response.indexOf(char(0x03));
    QString outResponse = QString::fromUtf8(response.mid(2, endIndex-2));
    
    //send to functions
    if(activeQuery == QString("PUMPSTATUSCODE")){
        responsePumpStatusCode(outResponse);
    }else if(activeQuery == QString("PUMPERRORCODE")){
        responsePumpErrorCode(outResponse);
    }else if(activeQuery == QString("PUMPTEMPERATURE")){
        responsePumpTemperature(outResponse);
    }else if(activeQuery == QString("PUMPDRIVE")){
        responsePumpDrive(outResponse);
    }else{
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Unknown activeQuery").arg(device_name));
        emit device_fail();
    }
       
    deviceStatus = QString("Status_%1_%2_%3_%4_%5").arg(device_name).arg(pumpStatusCode).arg(pumpErrorCode).arg(pumpTemperature).arg(pumpDrive);
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
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: No connection between controller and pump").arg(device_name));
    }
    if((pumpErrorCode&0x0002)==0x0002){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Pump is over maximum temperature").arg(device_name));
    }
    if((pumpErrorCode&0x0004)==0x0004){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Controller is over maximum temperature").arg(device_name));
    }
    if((pumpErrorCode&0x0010)==0x0010){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Power failure").arg(device_name));
    }
    if((pumpErrorCode&0x0020)==0x0020){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Auxilary failure").arg(device_name));
    }
    if((pumpErrorCode&0x0040)==0x0040){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Overvoltage").arg(device_name));
    }
    if((pumpErrorCode&0x0080)==0x0080){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Short circuit").arg(device_name));
    }
    if((pumpErrorCode&0x0100)==0x0100){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Pumping load is too high").arg(device_name));
    }
}

void AgilentTV301Pump::responsePumpTemperature(QString response)
{
    bool conv_ok;
    uint temperature = response.toUInt(&conv_ok);
    
    //check the temperature
    if(!conv_ok || temperature > 70){
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Pump response %2 is invalid for PUMPTEMPERATURE query").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    pumpTemperature = temperature;
    
    emit device_message(QString("Local AgilentTV301Pump: %1: Pump temperature: %2").arg(device_name).arg(pumpTemperature));
}

void AgilentTV301Pump::responsePumpDrive(QString response)
{
    bool conv_ok;
    uint drive = response.toUInt(&conv_ok);
    
    //check the drive
    if(!conv_ok){
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Pump response %2 is invalid for PUMPDRIVE query").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    pumpDrive = drive;
    
    emit device_message(QString("Local AgilentTV301Pump: %1: Pump drive: %2").arg(device_name).arg(pumpDrive));
}
