#include "include/AgilisMirrors.h"

AgilisMirrors::AgilisMirrors(QString deviceName, CascConfig * config, QObject * parent):
SerialDevice(deviceName, config, parent),
travelRange(14400),
currentAxis(1),
currentChannel(1),
// calibrating(false),
limitSign(0),
moving(false)
{
    if(device_failed)
        return;
    
    //Settings for serial communication with the pump over usb
    setBaudRate(921600);
    setDataBits(8);
	setParity(0);//No parity
	setStopBits(1);
	setFlowControl(1);//NoFlowControl
    
    setSerialTimeout(70);
    setSerialResponseWait(5);
    
    //response from device
    connect(this, SIGNAL(newSerialResponse(QByteArray)), this, SLOT(dealWithResponse(QByteArray)));
    
    //communication finished, take next command
    connect(this, &SerialDevice::serialComFinished, this, &AgilisMirrors::mirrorCommand);
    
    if(!openSerialPort())
        return;
    
    //assumes axes are not at limits initially
    QString status = "";
    for(int i=0; i<AGILIS_MIRRORS_N_CHANNELS*2; i++){
        // axisLimitSwitches[i] = false;
        // stepRange[i] = 0;
        axisPos[i] = 0;
        if(i!=AGILIS_MIRRORS_N_CHANNELS*2-1){
            status.append(QString("%1_").arg(axisPos[i]));
        }else{
            status.append(QString("%1").arg(axisPos[i]));
        }
    }
    setStatus(status);
    
    //put the mirror controller in remote mode
    queueSerialCommand("SETREMOTE");
    // queueSerialCommand("QUERYERROR");
    queueSerialCommand("QUERYCHANNEL");
    // queueSerialCommand("QUERYERROR");
    // queueSerialCommand("QUERYSTATUS_1");
    // queueSerialCommand("QUERYERROR");
    // queueSerialCommand("QUERYSTATUS_2");
    // queueSerialCommand("QUERYERROR");
}

// //set of commands to calibrate the axis step size
// //1 <= axis <= 8
// void AgilisMirrors::calibrate(uint axis)
// {
    // //queue to sequence of calibration commands, then call mirrorCommand() again to execute these commands
    // uint channelToCalibrate = (axis/2)+axis%2;//1,2,3,4
    // uint axisToCalibrate = (axis+1)%2+1;//1,2
    
    // if(channelToCalibrate != currentChannel){
        // queueSerialCommand(QString("SETCHANNEL_%1").arg(channelToCalibrate));
        // queueSerialCommand("QUERYERROR");
    // }
    
    // queueSerialCommand(QString("MOVENEGLIMIT_%1_%4").arg(axisToCalibrate).arg(3));//1700 steps/s at max step amplitude
    // // queueSerialCommand("QUERYERROR");
    // queueSerialCommand("QUERYSTATUS");
    // nextCalibrationCommand = "QUERYLIMITSTATUS";
    
    // // keep querying the limit status until it reaches the limit
    // // queueSerialCommand(QString("ZEROSTEPCOUNTER_%1").arg(axisToCalibrate));
    // // queueSerialCommand("QUERYERROR");
    // // queueSerialCommand(QString("MOVEREL_%1_%2").arg(axisToCalibrate).arg(100));//Move 100 steps to get out of the limit
    // // queueSerialCommand("QUERYERROR");
    // // query status
    
    // //keep querying the status until it stops and returns 'Ready' i.e. statusCode==0
    // // queueSerialCommand(QString("MOVEPOSLIMIT_%1_%4").arg(axisToCalibrate).arg(4));//666 steps/s at defined step amplitude
    // // queueSerialCommand("QUERYERROR");
    // // queueSerialCommand("QUERYLIMITSTATUS");
    // // queueSerialCommand("QUERYERROR");
    
    // // keep querying the limit status until it reaches the positive limit
    // // queueSerialCommand(QString("QUERYNUMSTEPS_%1").arg(axisToCalibrate));

    // calibrating = true;
// }


//dealing with queued commands
///////////////////////////////////////////////////////////////////////////////
void AgilisMirrors::mirrorCommand()
{
    //no further commands
    if(serialCommandQueue.isEmpty())
        return;
    
    QString command = serialCommandQueue.dequeue();    
    
    // emit device_message(QString("AgilisMirrors: Command %1").arg(command));
    
    QStringList command_list = command.split("_");
    
    if(command_list.length() == 1){
        mirrorSingleWordCommand(command);
    }else if(command_list.first() == "Command" && command_list.length() == 3){
        mirrorWidgetCommand(command_list);
    }else if(command_list.length() == 2){
        mirrorTwoWordCommand(command_list);
    }else if(command_list.length() == 3){
        mirrorThreeWordCommand(command_list);
    }
}

void AgilisMirrors::mirrorSingleWordCommand(QString command)    
{    //commands from within the device
    if(command == QString("QUERYERROR")){
        writeCommand(QString("TE?\r\n").toUtf8(), true);
    }else if(command == QString("QUERYLIMITSTATUS")){
        writeCommand(QString("1PH?\r\n").toUtf8(), true);
    }else if(command == QString("SETREMOTE")){
        writeCommand(QString("MR\r\n").toUtf8(), false);
    }else if(command == QString("QUERYCHANNEL")){
        writeCommand(QString("CC?\r\n").toUtf8(), true);
    }
    
    //prevent infinite loop
    if(command != QString("QUERYERROR")){
        queueSerialCommand("QUERYERROR");
    }
}

void AgilisMirrors::mirrorWidgetCommand(QStringList command_list)
{
    //check the epics variable name
    QStringList paramList = command_list.at(1).split(":");
    if(paramList.first() != "CASC" || paramList.length() < 3){
        return;
    }
    if(paramList.at(1) != device_name){
        return;
    }
    
    bool conv_ok;
    if(paramList.at(2) == "StopCommanded" && command_list.at(2) == "Stop"){//enum EPICS variable
        writeCommand(QString("%1ST\r\n").arg(currentAxis).toUtf8(), false);
        queueSerialCommand("QUERYERROR");
        // calibrating = false;
    // }else if(paramList.at(2) == "CalibrateCommanded"){//int EPICS variable
        // uint axisToCalibrate = command_list.at(2).toUInt(&conv_ok);
        // if(!conv_ok || axisToCalibrate < 1 || axisToCalibrate > 8){
            // emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid axis to calibrate").arg(device_name));
            // return;
        // }
        // calibrate(axisToCalibrate);
    }else if(paramList.at(2).startsWith("Jog")){
        uint axisToJog = paramList.at(2).mid(3).toUInt(&conv_ok);
        int jog = command_list.at(2).toInt(&conv_ok);
        if(!conv_ok || axisToJog < 1 || axisToJog > 8 || jog > 4 || jog < -4){
            emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid axis or speed to jog").arg(device_name));
            return;
        }
        
        uint channel = (axisToJog/2)+axisToJog%2;//1/2=0+1%2=1 => 1, 2/2=1+2%2=0 => 1, 3/2=1+3%2=1 => 2
                
        if(channel != currentChannel){
            queueSerialCommand(QString("SETCHANNEL_%1").arg(channel));
            // queueSerialCommand("QUERYERROR");
        }
        
        uint axis = (axisToJog+1)%2+1;//(1+1)%2=0+1 => 1, (4+1)%2=1+1 => 2
        queueSerialCommand(QString("SETJOG_%1_%2").arg(axis).arg(jog));
        // queueSerialCommand("QUERYERROR");
        queueSerialCommand(QString("QUERYSTATUS_%1").arg(axis));
        queueSerialCommand(QString("QUERYNUMSTEPS_%1").arg(currentAxis));
    }else{
        return;
    }
}

void AgilisMirrors::mirrorTwoWordCommand(QStringList command_list)
{
    bool conv_ok;
    uint value = command_list.at(1).toUInt(&conv_ok);
    if(!conv_ok){
        return;
    }
    
    uint channelAxis = (currentChannel-1)*2+currentAxis-1;
    
    if(command_list.first() == QString("QUERYSTATUS")){
        //check axis
        if(value == 0 || value > 2){
            return;
        }
        if(writeCommand(QString("%1TS?\r\n").arg(value).toUtf8(), true)){
            currentAxis = value;
        }
    }else if(command_list.first() == "QUERYNUMSTEPS" && command_list.length() == 2){
        //check axis
        if(value == 0 || value > 2){
            return;
        }
        // if((calibrating && axisLimitSwitches[channelAxis]) || !calibrating){
        if(writeCommand(QString("%1TP?\r\n").arg(value).toUtf8(), true)){
            currentAxis = value;
        }
        // }else{
            // return;
        // }
    }else if(command_list.first() == QString("SETCHANNEL")){
        //check channel
        if(value == 0 || value > 4){
            return;
        }
        if(writeCommand(QString("CC%1\r\n").arg(value).toUtf8(), false)){
            currentChannel = value;
        }
    }else if(command_list.first() == "ZEROSTEPCOUNTER"){
        //check axis
        if(value == 0 || value > 2){
            return;
        }
        //If calibrating, only set the zero point once we have reached the extreme
        // if((calibrating && axisLimitSwitches[channelAxis]) || !calibrating){
        if(writeCommand(QString("%1ZP\r\n").arg(value).toUtf8(), false)){
            currentAxis = value;
        }
        // }else{
            // return;
        // }
    }
    
    queueSerialCommand("QUERYERROR");
}

void AgilisMirrors::mirrorThreeWordCommand(QStringList command_list)
{
    bool conv_ok;
    uint axis = command_list.at(1).toUInt(&conv_ok);
    int param = command_list.at(2).toInt(&conv_ok);
    if(!conv_ok || axis == 0 || axis>2){
        return;
    }
    
    // uint channelAxis = (currentChannel-1)*2+currentAxis-1;//0 to 7
    
    QString toWrite;
    if(command_list.first() == "SETJOG"){
        if(param > 4 || param < -4){
            return;
        }
        toWrite = QString("%1JA%2\r\n").arg(axis).arg(param);
    }else if(command_list.first() == "MOVEREL"){
        toWrite = QString("%1PR%2\r\n").arg(axis).arg(param);
        if(param > 0){
            limitSign = 1;
        }else if(param < 0){
            limitSign = -1;
        }
    }else if(command_list.first() == "SETSTEP"){
        if(param < -50 || param > 50){
            return;
        }        
        toWrite = QString("%1SU%2\r\n").arg(axis).arg(param);
    }else if(command_list.first() == "MOVENEGLIMIT"){
        if(param < 0 || param > 4){
            return;
        }
        toWrite = QString("%1MV-%2\r\n").arg(axis).arg(param);
        limitSign = -1;
    }else if(command_list.first() == "MOVEPOSLIMIT"){
        if(param < 0 || param > 4){
            return;
        }
        toWrite = QString("%1MV%2\r\n").arg(axis).arg(param);
        limitSign = 1;
    }else{
        return;
    }
    
    if(writeCommand(toWrite.toUtf8(), false)){
        currentAxis = axis;
    }
    
    queueSerialCommand("QUERYERROR");
}

//response handling
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AgilisMirrors::dealWithResponse(QByteArray resp)
{
    //no response was received before the timeout, do nothing
    if(resp == noResponseMessage){
        return;
    }
    
    QString responseFull = QString::fromUtf8(resp);
    
    // emit device_message(QString("Agilis Mirrors: %1: response %2").arg(device_name).arg(responseFull));
    
    QStringList responseList = responseFull.split("\r\n");
    
    for(int i=0; i<responseList.length(); i++){
        QString response = responseList.at(i);
        if(response.size() >= 3){
            QString responsePart = response.mid(2);
            
            if(response.startsWith("TE")){
                responseErrorCode(responsePart);
            }else if(response.contains("TS")){
                responseAxisStatus(response);
            }else if(response.startsWith("CC")){
                responseChannel(responsePart);
            }else if(response.startsWith("PH")){
                responseLimitStatus(responsePart);
            }else if(response.contains("TP")){
                responseNumSteps(response);
            }else{
                emit device_message(QString("AgilisMirrors: %1: Invalid response %2").arg(device_name).arg(response));
            }
        }
    }
    
    //free up the serial device to take the next command
    fullResponseReceived();
}

void AgilisMirrors::responseErrorCode(QString response)
{
    bool conv_ok;
    int errorCode = response.toInt(&conv_ok);
    
    if(!conv_ok || errorCode > 0 || errorCode < -6){
        emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid response to error code query, %2").arg(device_name).arg(response));
    }else if(errorCode != 0){
        emit device_fail();
        switch(errorCode){
            case -1:
                emit device_message(QString("AGILIS MIRRORS ERROR: %1: Error code: Unknown command").arg(device_name));
                break;
            case -2:
                emit device_message(QString("AGILIS MIRRORS ERROR: %1: Error code: Axis incorrectly specified").arg(device_name));
                break;
            case -3:
                emit device_message(QString("AGILIS MIRRORS ERROR: %1: Error code: Wrong format for command parameter").arg(device_name));
                break;
            case -4:
                emit device_message(QString("AGILIS MIRRORS ERROR: %1: Error code: Command parameter out of range").arg(device_name));
                break;
            case -5:
                emit device_message(QString("AGILIS MIRRORS ERROR: %1: Error code: Forbidden in local mode").arg(device_name));
                break;
            case -6:
                emit device_message(QString("AGILIS MIRRORS ERROR: %1: Error code: Forbidden in current state").arg(device_name));
                break;
        }
    }
}

void AgilisMirrors::responseAxisStatus(QString response)
{
    bool conv_ok;
    QStringList responseParts = response.split("TS");
    
    uint axis = 0; 
    uint statusCode = 5;
    if(responseParts.length() == 2){
        axis = responseParts.first().toUInt(&conv_ok);
        statusCode = responseParts.at(1).toUInt(&conv_ok);
        
        currentAxis = axis;
    }else{
        conv_ok = false;
    }
    
    if(!conv_ok || statusCode > 4){
        emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid response to axis status query, %2").arg(device_name).arg(response));
    }else{
        // uint channelAxis = (currentChannel-1)*2+currentAxis-1;//0 to 7
        
        // //keep querying the status until the mirror has stopped and is ready
        // if(calibrating && statusCode == 0){
            // queueSerialCommand(nextCalibrationCommand);
            // queueSerialCommand("QUERYSTATUS");
            
            // if(nextCalibrationCommand == "QUERYLIMITSTATUS" && limitSign < 0){
                // nextCalibrationCommand = QString("MOVEPOSLIMIT_%1_%4").arg(currentAxis).arg(4);
            // }else if(nextCalibrationCommand == QString("MOVEPOSLIMIT_%1_%4").arg(currentAxis).arg(4)){
                // nextCalibrationCommand = "QUERYLIMITSTATUS";
            // }
        // // }else if(statusCode != 0){
            // // queueSerialCommand(QString("QUERYSTATUS_%1").arg(currentAxis));
        // }
        
        if(statusCode != 0){
            moving = true;
        }else{
            moving = false;
        }
    }
}

void AgilisMirrors::responseChannel(QString response)
{
    bool conv_ok;
    uint channel = response.toUInt(&conv_ok);
    
    if(!conv_ok || channel == 0 || channel > 4){
        emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid response to channel query, %2").arg(device_name).arg(response));
    }else{
        currentChannel = channel;
    }
}

//number of steps taken either since zeroing the step count or since powering on the mirror
void AgilisMirrors::responseNumSteps(QString response)
{
    bool conv_ok;
    uint axis = 0;
    int steps = 1;
    QStringList responseParts = response.split("TP");
    if(responseParts.length() == 2){
        axis = responseParts.first().toUInt(&conv_ok);
        steps = responseParts.at(1).toInt(&conv_ok);
        
        currentAxis = axis;
    }else{
        conv_ok = false;
    }
    
    uint channelAxis = (currentChannel-1)*2+currentAxis-1;//0 to 7
    // if(conv_ok && calibrating){
        // //positive limit in terms of number of steps
        // stepRange[channelAxis] = steps;
    // }
    
    if(!conv_ok){
        emit device_message(QString("AgilisMirrors: %1: Invalid response to number of steps query, %2").arg(device_name).arg(response));
    }else{
        //position relative to negative limit in arc seconds as travel range = 14400 = 4 degrees in arc seconds
        // if(stepRange[channelAxis] == 0){
        axisPos[channelAxis] = steps;
        // }else{
            // //calibrated
            // axisPos[channelAxis] = (steps*travelRange)/stepRange[channelAxis];
        // }
        
        //set the status positions as one has changed
        QString status = "";
        for(int i=0; i<AGILIS_MIRRORS_N_CHANNELS*2; i++){
            if(i!=AGILIS_MIRRORS_N_CHANNELS*2-1){
                status.append(QString("%1_").arg(axisPos[i]));
            }else{
                status.append(QString("%1").arg(axisPos[i]));
            }
        }
        // emit device_message(QString("AgilisMirrors: Status %1").arg(status));
        setStatus(status);
    }

    //continuously probe the position while moving
    if(moving){
        queueSerialCommand(QString("QUERYNUMSTEPS_%1").arg(currentAxis));
    }    
}

void AgilisMirrors::responseLimitStatus(QString response)
{
    bool conv_ok;
    uint limitStatusCode = response.toUInt(&conv_ok);
    if(!conv_ok || limitStatusCode > 3){
        emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid response to limit status query, %2").arg(device_name).arg(response));
        return;
    }
    
    // uint axis1 = (currentChannel-1)*2;//0,2,4,6
    // uint axis2 = currentChannel*2-1;//1,3,5,7
    
    // switch(limitStatusCode){
        // case 0:
            // axisLimitSwitches[axis1] = false;
            // axisLimitSwitches[axis2] = false;
            // break;
        // case 1:
            // axisLimitSwitches[axis1] = true;
            // axisLimitSwitches[axis2] = false;
            // break;
        // case 2:
            // axisLimitSwitches[axis1] = false;
            // axisLimitSwitches[axis2] = true;
            // break;
        // case 3:
            // axisLimitSwitches[axis1] = true;
            // axisLimitSwitches[axis2] = true;
            // break;
    // }
    
    // //When calibrating, keep querying the axis limit until at the limit
    // uint channelAxis = (currentChannel-1)*2+currentAxis-1;//0 to 7
    // if(calibrating && axisLimitSwitches[channelAxis]){
        // if(limitSign < 0){//at the negative limit while calibrating
            // queueSerialCommand(QString("ZEROSTEPCOUNTER_%1").arg(currentAxis));
            // queueSerialCommand(QString("MOVEREL_%1_%2").arg(currentAxis).arg(100));//Move 100 steps to get out of the negative limit
            // queueSerialCommand(QString("QUERYSTATUS_%1").arg(currentAxis));
        // }else{
            // queueSerialCommand(QString("QUERYNUMSTEPS_%1").arg(currentAxis));
        // }
    // }
}

