#include "include/LaseLock.h"

LaseLock::LaseLock(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(file_path, file_mutex, deviceName, config, parent),
// queryTimeout(200),
queryNumber(0),
activeQuery(QString("NONE")),
regALocked(2),
regBLocked(2),
regASearch(2),
regBSearch(2),
regARelock(2),
regBRelock(2),
regAInClip(2),
regBInClip(2),
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
    // QTimer * queryTimer = new QTimer;
    // queryTimer->setInterval(queryTimeout);
    // connect(queryTimer, SIGNAL(timeout()), this, SLOT(queryNextNumber()));
    // connect(this, SIGNAL(device_fail()), queryTimer, SLOT(stop()));
    connect(this, &SerialDevice::serialComFinished, this, &LaseLock::queryNextNumber);
    
    if(!openSerialPort())
        return;
    
    //initial commands
    queryNextNumber();

    //start the query timers
    // queryTimer->start();
    
    //set the status
    //replaced Relock with InClip
    deviceStatus = QString("Status_%1_%2_%3_%4_%5_%6_%7_%8_%9").arg(device_name).arg(regALocked).arg(regBLocked).arg(regASearch).arg(regBSearch).arg(regAInClip).arg(regBInClip).arg(regAHold).arg(regBHold);
}

//query all of the commands at the same rate in sequence
void LaseLock::queryNextNumber()
{
    if(device_failed)
        return;
    
    //REGARELOCK, REGBRELOCK and REGBLOCKED not working
    QStringList querysAsNumbers = {"REGALOCKED", "REGBLOCKED", "REGASEARCH", "REGBSEARCH", "REGAINCLIP", "REGBINCLIP", "REGAHOLD", "REGBHOLD"};
    
    if(queryNumber >= querysAsNumbers.length()){
        queryNumber = 0;
    }
    
    // emit device_message(QString("Local LaseLock: queryNumber %1").arg(queryNumber));
    queueSerialCommand(querysAsNumbers.at(queryNumber));
    queryNumber++;     
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
    }else if(command_list.first() == QString("REGAINCLIP")){
        toQuery = QString("InClipA=\r");
    }else if(command_list.first() == QString("REGBINCLIP")){
        toQuery = QString("InClipB=\r");
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
        activeQuery = toQuery;
}

void LaseLock::dealWithResponse(QByteArray resp)
{
    //no response was received before the timeout, do nothing
    if(resp == noResponseMessage){
        activeQuery = QString("NONE");
        return;
    }
    
    QString response = QString::fromUtf8(resp);
    
    // emit device_message(QString("Local LaseLock: %1: Response: %2").arg(device_name).arg(response));
    
    //error message from device
    if(response.contains("ERROR")){
        emit device_message(QString("LOCAL LASELOCK ERROR: %1: %2").arg(device_name).arg(response));
        emit device_fail();
        activeQuery = QString("NONE");
        return;
    }
    
    //not expecting a response
    if(activeQuery == QString("NONE")){
        return;
    }
    
    fullResponse.append(response);
    
    //check not receiving last bit of last query
    if(!fullResponse.startsWith(activeQuery.at(0))){
        fullResponse.clear();
        return;
    }
    
    //check the full response has been received
    //ONLY WORKS FOR BOOLEANS
    /////////////////////////////////////////////////////////////////////////////
    if(!fullResponse.contains("=") || (!fullResponse.contains("0") && !fullResponse.contains("1"))){
        return;
    }
    ////////////////////////////////////////////////////////////////////////////
    
    //full response has been received
    response = fullResponse;
    fullResponse.clear();
    
    //check the response corresponds to the query
    QStringRef subResp(&response, 0,response.indexOf("="));
    QStringRef subQuery(&activeQuery, 0,activeQuery.indexOf("="));
    if(subResp != subQuery || response.length() == response.indexOf("=")+1){
        emit device_message(QString("Local LaseLock: %1: Pump response %2 is invalid for query %3").arg(device_name).arg(response).arg(activeQuery));
        activeQuery = QString("NONE");
        fullResponseReceived();
        return;
    }
    
    QString outResponse = response.mid(response.indexOf("=")+1);
    
    if(activeQuery == QString("LockedA=\r")){
        checkBoolResponse(outResponse, &regALocked);
    }else if(activeQuery == QString("LockedB=\r")){
        checkBoolResponse(outResponse, &regBLocked);
    }else if(activeQuery == QString("SearchA=\r")){
        checkBoolResponse(outResponse, &regASearch);
    }else if(activeQuery == QString("SearchB=\r")){
        checkBoolResponse(outResponse, &regBSearch);
    }else if(activeQuery == QString("RelockA=\r")){
        checkBoolResponse(outResponse, &regARelock);
    }else if(activeQuery == QString("RelockB=\r")){
        checkBoolResponse(outResponse, &regBRelock);
    }else if(activeQuery == QString("InClipA=\r")){
        checkBoolResponse(outResponse, &regAInClip);
    }else if(activeQuery == QString("InClipB=\r")){
        checkBoolResponse(outResponse, &regBInClip);
    }else if(activeQuery == QString("HoldA=\r")){
        checkBoolResponse(outResponse, &regAHold);
    }else if(activeQuery == QString("HoldB=\r")){
        checkBoolResponse(outResponse, &regBHold);
    }else if(activeQuery != QString("NONE")){
        emit device_message(QString("LOCAL LASELOCK ERROR: %1: Unknown activeQuery, %2").arg(device_name).arg(activeQuery));
        emit device_fail();
    }
    
    //set the status
    //removed relock and replaced with inclip
    deviceStatus = QString("Status_%1_%2_%3_%4_%5_%6_%7_%8_%9").arg(device_name).arg(regALocked).arg(regBLocked).arg(regASearch).arg(regBSearch).arg(regAInClip).arg(regBInClip).arg(regAHold).arg(regBHold);
    
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
        emit device_message(QString("Local LaseLock: %1: Response %2 is invalid for boolean query").arg(device_name).arg(response));
    }else{
        *flag = bool_val;
    }
}