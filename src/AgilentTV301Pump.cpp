#include "include/AgilentTV301Pump.h"

// AgilentTV301Pump::AgilentTV301Pump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
AgilentTV301Pump::AgilentTV301Pump(QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(deviceName, config, parent),
statusTimer(new QTimer(this)),
statusTimeout(1000),
errorCodeTimer(new QTimer(this)),
errorCodeTimeout(1010),
temperatureTimer(new QTimer(this)),
temperatureTimeout(1020),
driveTimer(new QTimer(this)),
driveTimeout(1030),
pumpStatus(QString("off")),
pumpError(QString("off")),
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
    statusTimer->setInterval(statusTimeout);
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(queryStatus()));
    connect(this, SIGNAL(device_fail()), statusTimer, SLOT(stop()));
    
    errorCodeTimer->setInterval(errorCodeTimeout);
    connect(errorCodeTimer, SIGNAL(timeout()), this, SLOT(queryErrorCode()));
    connect(this, SIGNAL(device_fail()), errorCodeTimer, SLOT(stop()));
    
    temperatureTimer->setInterval(temperatureTimeout);
    connect(temperatureTimer, SIGNAL(timeout()), this, SLOT(queryTemperature()));
    connect(this, SIGNAL(device_fail()), temperatureTimer, SLOT(stop()));
    
    driveTimer->setInterval(driveTimeout);
    connect(driveTimer, SIGNAL(timeout()), this, SLOT(queryDrive()));
    connect(this, SIGNAL(device_fail()), driveTimer, SLOT(stop()));

    ///////////////////////////////////////////////////////////////////////////////////////////////
    if(!openSerialPort())
        return;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    
    //initial commands
    // queueSerialCommand(QString("PUMPSTATUSCODE"));
    // queueSerialCommand(QString("PUMPERRORCODE"));
    // queueSerialCommand(QString("PUMPTEMPERATURE"));
    // queueSerialCommand(QString("PUMPDRIVE"));

    //periodic commands timers
    statusTimer->start();
    errorCodeTimer->start();
    temperatureTimer->start();
    driveTimer->start();
    
    //intial device status: device_name, statusCode, errorCode, temperature
    setStatus(QString("%1_%2_%3_%4").arg(pumpStatus).arg(pumpError).arg(pumpTemperature).arg(pumpDrive)); 
    
    //save the first real values to come in
    // saveToFile = true;
}

void AgilentTV301Pump::queryStatus()
{
    queueSerialCommand(QString("PUMPSTATUSCODE"));
}

void AgilentTV301Pump::queryErrorCode()
{
    queueSerialCommand(QString("PUMPERRORCODE"));
}

void AgilentTV301Pump::queryTemperature()
{
    queueSerialCommand(QString("PUMPTEMPERATURE"));
}

void AgilentTV301Pump::queryDrive()
{
    queueSerialCommand(QString("PUMPDRIVE"));
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
    //crc is 0x84 => ascii 0x38, 0x34
    if(command_list.first() == QString("PUMPSTATUSCODE")){
        toQuery[0] = 0x02;
        toQuery[1] = 0x80;
        toQuery[2] = 0x32;
        toQuery[3] = 0x30;
        toQuery[4] = 0x35;
        toQuery[5] = 0x30;
        toQuery[6] = 0x03;
        toQuery[7] = 0x38;
        toQuery[8] = 0x34;       
    //crc is 0x87 => ascii 0x38, 0x37
    }else if(command_list.first() == QString("PUMPERRORCODE")){
        toQuery[0] = 0x02;
        toQuery[1] = 0x80;
        toQuery[2] = 0x32;
        toQuery[3] = 0x30;
        toQuery[4] = 0x36;
        toQuery[5] = 0x30;
        toQuery[6] = 0x03;
        toQuery[7] = 0x38;
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
    
    // //send the query
    // if(activeQuery != QString("NONE")){
        // emit device_message(QString("Local AgilentTV301Pump: %1: Busy waiting for reply").arg(device_name));
        // return;
    // }
    
    // QString hexValues = getHexValues(toQuery);
    // storeMessage(QString("Local AgilentTV301Pump: %1: Sending query %2").arg(device_name).arg(hexValues), false);
    
    writeCommand(toQuery, true);
}

QString AgilentTV301Pump::getHexValues(QByteArray values)
{
    QString hexValues = "";
    for(int i=0; i<values.length(); i++){
        ushort charac = ushort(uchar(values.at(i)));
        hexValues.append(QString("0x%1,").arg(charac,0,16));
    }
    return hexValues;
}

//need to deal with multiple parts of response coming at different times
void AgilentTV301Pump::dealWithResponse(QByteArray response)
{
    //no response was received before the timeout, do nothing
    if(response == noResponseMessage){
        // activeQuery = QString("NONE");
        return;
    }
    
    // emit device_message(QString("Local AgilentTV301Pump: %1: Response: %2").arg(device_name).arg(getHexValues(response)));
    
    // if(activeQuery == QString("NONE")){
        // return;
    // }
    
    fullResponse.append(response);
    
    //check not left over bit from last query
    if(!fullResponse.startsWith(char(0x02))){
        fullResponse.clear();
        return;
    }
    
    //check the whole response has been received
    if(fullResponse.length() < 6 || !fullResponse.contains(char(0x03))){
        // emit device_message(QString("Local AgilentTV301Pump: %1: Pump response %2 is invalid for query %3").arg(device_name).arg(getHexValues(fullResponse)).arg(activeQuery));
        return;
    }
    
    response = fullResponse;
    fullResponse.clear();

    int endIndex = response.indexOf(char(0x03));
    
    //NAK response
    QByteArray interestingResponse = response.mid(2, endIndex-2);
    if(interestingResponse == QByteArray(1, 0x15)){
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: The execution of the last command failed").arg(device_name));
        emit device_fail();
        return;
    }else if(interestingResponse == QByteArray(1, 0x32)){
        emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: The window for the last command is not valid").arg(device_name));
        emit device_fail();
        return;
    }
    
    QString outResponse = QString::fromUtf8(interestingResponse);
    
    //check length
    if(outResponse.length() != 10){
        emit device_message(QString("Local AgilentTV301Pump: %1: Pump response %2 is invalid").arg(device_name).arg(outResponse));
    }else if(outResponse.startsWith("205")){
        responsePumpStatusCode(outResponse);
    }else if(outResponse.startsWith("206")){
        responsePumpErrorCode(outResponse);
    }else if(outResponse.startsWith("204")){
        responsePumpTemperature(outResponse);
    }else if(outResponse.startsWith("203")){
        responsePumpDrive(outResponse);
    }
       
    setStatus(QString("%1_%2_%3_%4").arg(pumpStatus).arg(pumpError).arg(pumpTemperature).arg(pumpDrive));
    // emit device_message(QString("Local AgilentTV301Pump: %1: device status %2").arg(device_name).arg(deviceStatus));
    
    // activeQuery = QString("NONE");
    //free up the serial device
    fullResponseReceived();    
}


//pump responses
/////////////////////////////////////////////////////////////////////////////
void AgilentTV301Pump::responsePumpStatusCode(QString response)
{
    response = response.mid(3);
    
    bool conv_ok;
    uint code = response.toUInt(&conv_ok);
    
    //check the code
    if(!conv_ok || code > 6){
        emit device_message(QString("Local AgilentTV301Pump: %1: Pump response %2 is invalid for PUMPSTATUSCODE query").arg(device_name).arg(response));
    }else{
        switch(code){
            case 0:
                pumpStatus = "Stopped";
                break;
            case 1:
                pumpStatus = "Interlock";
                break;
            case 2:
                pumpStatus = "Starting";
                break;
            case 3:
                pumpStatus = "Tuning";
                break;
            case 4:
                pumpStatus = "Braking";
                break;
            case 5:
                pumpStatus = "Running";
                break;
            case 6:
                pumpStatus = "FAULT";
                break;
        }
    }
    // emit device_message(QString("Local AgilentTV301Pump: %1: Pump status code: %2").arg(device_name).arg(pumpStatusCode));
}

void AgilentTV301Pump::responsePumpErrorCode(QString response)
{
    // //check length
    // if(response.length() != 10){
        // emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Pump response %2 is invalid for PUMPERRORCODE query").arg(device_name).arg(response));
        // emit device_fail();
        // return;
    // }    
    // //response starts with the window accessed
    // if(!response.startsWith("206")){
        // emit device_message(QString("LOCAL AGILENTTV301PUMP ERROR: %1: Pump response %2 is invalid for PUMPERRORCODE query").arg(device_name).arg(response));
        // emit device_fail();
        // return;
    // }
    response = response.mid(3);
    
    bool conv_ok;
    uint code = response.toUInt(&conv_ok);
    
    //check the code
    if(!conv_ok || code > 255){
        emit device_message(QString("Local AgilentTV301Pump: %1: Pump response %2 is invalid for PUMPERRORCODE query").arg(device_name).arg(response));
        return;
    }
        
    // emit device_message(QString("Local AgilentTV301Pump: %1: Pump error code: %2").arg(device_name).arg(pumpErrorCode));
    
    if(code == 0){
        pumpError = "ok";
        return;
    }else{
        pumpError = "FAULT";
    }
    
    //error codes decoded
    if((code&0x0001)==0x0001){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: No connection between controller and pump").arg(device_name));
    }
    if((code&0x0002)==0x0002){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Pump is over maximum temperature").arg(device_name));
    }
    if((code&0x0004)==0x0004){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Controller is over maximum temperature").arg(device_name));
    }
    if((code&0x0010)==0x0010){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Power failure").arg(device_name));
    }
    if((code&0x0020)==0x0020){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Auxilary failure").arg(device_name));
    }
    if((code&0x0040)==0x0040){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Overvoltage").arg(device_name));
    }
    if((code&0x0080)==0x0080){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Short circuit").arg(device_name));
    }
    if((code&0x0100)==0x0100){
        emit device_message(QString("Local AgilentTV301Pump: %1: FAULT: Pumping load is too high").arg(device_name));
    }
}

void AgilentTV301Pump::responsePumpTemperature(QString response)
{
    response = response.mid(3);
    
    bool conv_ok;
    uint temperature = response.toUInt(&conv_ok);
    
    //check the temperature
    if(!conv_ok || temperature > 70){
        emit device_message(QString("Local AgilentTV301Pump: %1: Pump response %2 is invalid for PUMPTEMPERATURE query").arg(device_name).arg(response));
    }else{
        pumpTemperature = temperature;
    }
    // emit device_message(QString("Local AgilentTV301Pump: %1: Pump temperature: %2").arg(device_name).arg(pumpTemperature));
}

void AgilentTV301Pump::responsePumpDrive(QString response)
{
    response = response.mid(3);
    
    bool conv_ok;
    uint drive = response.toUInt(&conv_ok);
    
    //check the drive
    if(!conv_ok){
        emit device_message(QString("Local AgilentTV301Pump: %1: Pump response %2 is invalid for PUMPDRIVE query").arg(device_name).arg(response));
    }else{
        pumpDrive = drive;
    }
    // emit device_message(QString("Local AgilentTV301Pump: %1: Pump drive: %2").arg(device_name).arg(pumpDrive));
}
