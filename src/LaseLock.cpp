#include "include/LaseLock.h"

LaseLock::LaseLock(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(QStringList({"LockedA", "LockedB", "SearchA", "SearchB", "InClipA", "InClipB", "HoldA", "HoldB"}), file_path, file_mutex, deviceName, config, parent),
queryNumber(0),
windows({"LockedA", "LockedB", "SearchA", "SearchB", "InClipA", "InClipB", "HoldA", "HoldB"}),
boolValues(windows.length(),2)//boolean values for the windows, initialise with 2=>no value
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

    //continuously send query commands
    connect(this, &SerialDevice::serialComFinished, this, &LaseLock::queryNextNumber);
    
    if(!openSerialPort())
        return;
    
    //set the status
    QString status = QString("");
    for(int i=0; i<boolValues.length()-1; i++){
        status.append(QString("%1").arg(boolValues.at(i)));
        status.append("_");
    }
    status.append(QString("%1").arg(boolValues.at(boolValues.length()-1)));
    setStatus(status);
    
    //save the first real values to come in
    saveToFile = true;
    
    //initial command, regA locked
    queryNextNumber();
}

//query all of the commands at the same rate in sequence
void LaseLock::queryNextNumber()
{
    if(device_failed)
        return;

    if(queryNumber >= windows.length()){
        // emit device_message(QString("Local LaseLock: %1: Command set finished").arg(device_name));
        queryNumber = 0;
    }
    
    // storeMessage(QString("Local LaseLock: queryNumber %1").arg(queryNumber), false);
    queueSerialCommand(windows.at(queryNumber));
    queryNumber++;     
}

void LaseLock::lockCommand()
{
    //no further commands
    if(serialCommandQueue.isEmpty())
        return;
    
    QString command = serialCommandQueue.dequeue();    
    QString toQuery;
    
    if(windows.contains(command)){
        toQuery = QString("%1=\r").arg(command);
    }else{
        return;
    }
    
    //send the query, SerialDevice rejects if busy    
    writeCommand(toQuery.toUtf8(), true);
}

// QString LaseLock::getHexValues(QByteArray values)
// {
    // QString hexValues = "";
    // for(int i=0; i<values.length(); i++){
        // ushort charac = ushort(uchar(values.at(i)));
        // hexValues.append(QString("0x%1,").arg(charac,0,16));
    // }
    // return hexValues;
// }

void LaseLock::dealWithResponse(QByteArray resp)
{
    //no response was received before the timeout, do nothing
    if(resp == noResponseMessage){
        return;
    }
    
    QString response = QString::fromUtf8(resp);
    
    // emit device_message(QString("Local LaseLock: %1: Response: %2").arg(device_name).arg(response));
    // emit device_message(QString("Local LaseLock: %1: Response: %2").arg(device_name).arg(getHexValues(resp)));
    
    //error message from device
    if(response.contains("ERROR")){
        emit device_message(QString("LOCAL LASELOCK ERROR: %1: %2").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    //multiple responses in one read, responses end with <CR><LF>
    QStringList response_list = response.split("\r\n");
    
    bool conv_ok;
    for(int i=0; i<response_list.length(); i++){
        //two parts of a response: the window and the value
        QStringList response_parts = response_list.at(i).split("=");
        
        //partial response
        if(response_parts.length()!=2){
            continue;
        }
        
        int responseIndex = windows.indexOf(response_parts.at(0));
        uint bool_val = response_parts.at(1).toUInt(&conv_ok);
        
        //partial response
        if(responseIndex<0 || !conv_ok || bool_val>1){
            continue;
        }else if(responseIndex == 0){
            emit device_message(QString("Local LaseLock: %1: Regulator A locked?: %2").arg(device_name).arg(bool_val));
        }
        //assign value to window
        boolValues[responseIndex] = bool_val;            
    }
        
    //set the status
    QString status = QString("");
    for(int i=0; i<boolValues.length()-1; i++){
        status.append(QString("%1").arg(boolValues.at(i)));
        status.append("_");
    }
    status.append(QString("%1").arg(boolValues.at(boolValues.length()-1)));
    setStatus(status);
    
    //free up the serial device
    fullResponseReceived();
}