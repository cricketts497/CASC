#include "include/AgilisMirrors.h"

AgilisMirrors::AgilisMirrors(QString deviceName, CascConfig * config, QObject * parent):
SerialDevice(deviceName, config, parent),
travelRange(4.0),
currentAxis(1),
currentChannel(1),
calibrating(false),
command(QString("NONE")),
limitSign(0)
{
    if(device_failed)
        return;
    
    //Settings for serial communication with the pump over usb
    setBaudRate(921600);
    setDataBits(8);
	setParity(0);//No parity
	setStopBits(1);
	setFlowControl(1);//NoFlowControl
    
    //response from device
    connect(this, SIGNAL(newSerialResponse(QByteArray)), this, SLOT(dealWithResponse(QByteArray)));
    
    //communication finished, take next command
    connect(this, &SerialDevice::serialComFinished, this, &AgilisMirrors::mirrorCommand);
    
    if(!openSerialPort())
        return;
    
    //assumes axes are not at limits initially
    for(int i=0; i<AGILIS_MIRRORS_N_CHANNELS*2; i++){
        axisLimitSwitches[i] = false;
        axisStatus[i] = 0;
        stepRange[i] = 0.0;
    }
    
    //put the mirror controller in remote mode
    queueSerialCommand("SETREMOTE");
    queueSerialCommand("QUERYERROR");
    queueSerialCommand("QUERYCHANNEL");
    queueSerialCommand("QUERYERROR");
}

//set of commands to calibrate the axis step size
//1 <= axis <= 8
void AgilisMirrors::calibrate(int axis)
{
    //queue to sequence of calibration commands, then call mirrorCommand() again to execute these commands
    int channelToCalibrate = axis/2;//1,2,3,4
    int axisToCalibrate = (axis+1)%2+1;//1,2
    
    if(channelToCalibrate != currentChannel){
        queueSerialCommand(QString("SETCHANNEL_%1").arg(channelToCalibrate));
        queueSerialCommand("QUERYERROR");
    }
    
    queueSerialCommand(QString("MOVENEGLIMIT_%1_%4").arg(axisToCalibrate).arg(3));//1700 steps/s at max step amplitude
    queueSerialCommand("QUERYERROR");
    queueSerialCommand("QUERYLIMITSTATUS");
    
    // keep querying the limit status until it reaches the limit
    // queueSerialCommand(QString("ZEROSTEPCOUNTER_%1").arg(axisToCalibrate));
    // queueSerialCommand("QUERYERROR");
    // queueSerialCommand(QString("MOVEREL_%1_%2").arg(axisToCalibrate).arg(100));//Move 100 steps to get out of the limit
    // queueSerialCommand("QUERYERROR");
    // query status
    
    //keep querying the status until it stops and returns 'Ready' i.e. statusCode==0
    // queueSerialCommand(QString("MOVEPOSLIMIT_%1_%4").arg(axisToCalibrate).arg(4));//666 steps/s at defined step amplitude
    // queueSerialCommand("QUERYERROR");
    // queueSerialCommand("QUERYLIMITSTATUS");
    // queueSerialCommand("QUERYERROR");
    
    // keep querying the limit status until it reaches the positive limit
    // queueSerialCommand(QString("QUERYNUMSTEPS_%1").arg(axisToCalibrate));

    calibrating = true;

    mirrorCommand();
}


//dealing with queued commands
///////////////////////////////////////////////////////////////////////////////
void AgilisMirrors::mirrorCommand()
{
    //no further commands
    if(serialCommandQueue.isEmpty())
        return;
    
    command = serialCommandQueue.dequeue();    
    
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
    //commands from within the device
    if(command == QString("QUERYERROR")){
        writeCommand(QString("TE?\r\n").toUtf8(), true);
    }else if(command == QString("QUERYLIMITSTATUS")){
        writeCommand(QString("PH?\r\n").toUtf8(), true);
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
    
    if(paramList.at(2) == "StopCommanded" && command_list.at(2) == "Stop"){//enum EPICS variable
        writeCommand(QString("%1ST\r\n").arg(currentAxis).toUtf8(), false);
    }else if(paramList.at(2) == "CalibrateCommanded"){//int EPICS variable
        int axisToCalibrate = command_list.at(2).toUInt(&conv_ok);
        if(!conv_ok || axisToCalibrate < 1 || axisToCalibrate > 8){
            emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid axis to calibrate").arg(device_name));
            return;
        }
        calibrate(axisToCalibrate);
    }else if(paramList.at(2) == "StepSizeCommanded"){
        //use hundreds for channel/axis and tens/digits for step size
        //could be positive or negative
        int stepAxis = command_list.at(2).toUInt(&conv_ok);
        if(!conv_ok || stepAxis < 100 || stepAxis > 850){
            emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid step size").arg(device_name));
            return;
        }
        
        int channel = (abs(stepAxis)/100)/2;
        int axis = ((abs(stepAxis)/100)+1)%2+1;
        int step = stepAxis%100;
        
        if(channel != currentChannel){
            queueSerialCommand(QString("SETCHANNEL_%1").arg(channel));
            queueSerialCommand("QUERYERROR");
        }
        queueSerialCommand(QString("SETSTEP_%1_%2").arg(axis).arg(step));
    }else{
        return;
    }
    
    queueSerialCommand("QUERYERROR");
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
        if(writeCommand(QString("%1TS\r\n").arg(value).toUtf8(), false)){
            currentAxis = value;
        }
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
        if((calibrating && axisLimitSwitches[channelAxis]) || !calibrating){
            if(writeCommand(QString("%1ZP\r\n").arg(value).toUtf8(), false)){
                currentAxis = value;
            }
        }else{
            return;
        }
    }else if(command_list.first() == "QUERYNUMSTEPS" && command_list.length() == 2){
        //check axis
        if(value == 0 || value > 2){
            return;
        }
        if((calibrating && axisLimitSwitches[channelAxis]) || !calibrating){
            if(writeCommand(QString("%1TP\r\n").arg(value).toUtf8(), true)){
                currentAxis = value;
            }
        }else{
            return;
        }
    }
    
    if(!calibrating){
        queueSerialCommand("QUERYERROR");
    }
}

void AgilisMirrors::mirrorThreeWordCommand(QStringList command_list)
{
    bool conv_ok;
    uint axis = command_list.at(1).toUInt(&conv_ok);
    int param = command_list.at(2).toInt(&conv_ok);
    if(!conv_ok || axis == 0 || axis>2){
        return;
    }
    
    uint channelAxis = (currentChannel-1)*2+currentAxis-1;//0 to 7
    
    QString toWrite;
    if(command_list.first() == "MOVEREL"){
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
    
    if(!calibrating){
        queueSerialCommand("QUERYERROR");
    }
}

//response handling
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AgilisMirrors::dealWithResponse(QByteArray resp)
{
    //no response was received before the timeout, do nothing
    if(resp == noResponseMessage){
        return;
    }
    
    QString response = QString::fromUtf8(resp);
    
    QStringList command_list = command.split("_");
    
    //send the response to the correct function for the last command/ query sent
    if(command == QString("QUERYERROR")){
        responseErrorCode(response);
    }else if(command_list.first() == QString("QUERYSTATUS")){
        responseAxisStatus(response);
    }else if(command == QString("QUERYCHANNEL")){
        responseChannel(response);
    }else if(command == QString("QUERYLIMITSTATUS")){
        responseLimitStatus(response);
    }else if(command_list.first() == QString("QUERYNUMSTEPS")){
        responseNumSteps(response);
    }else{
        emit device_message(QString("AgilisMirrors: %1: Received response %1 for command %2").arg(response).arg(command));
    }
    
    //set the status
    
    
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
    int statusCode = response.toUInt(&conv_ok);
    
    if(!conv_ok || statusCode > 4){
        emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid response to axis status query, %2").arg(device_name).arg(response));
    }else{
        uint channelAxis = (currentChannel-1)*2+currentAxis-1;//0 to 7
        axisStatus[channelAxis] = statusCode;
        
        //keep querying the status until the mirror has stopped and is ready
        if(calibrating && statusCode == 0 && limitSign < 0){
            queueSerialCommand(QString("MOVEPOSLIMIT_%1_%4").arg(currentAxis).arg(4));//666 steps/s at defined step amplitude
            queueSerialCommand("QUERYERROR");
            queueSerialCommand("QUERYLIMITSTATUS");
        }else if(statusCode != 0){
            queueSerialCommand(QString("QUERYSTATUS_%1").arg(currentAxis));
            queueSerialCommand("QUERYERROR");
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

void AgilisMirrors::responseLimitStatus(QString response)
{
    uint axis1 = (currentChannel-1)*2;//0,2,4,6
    uint axis2 = currentChannel*2-1;//1,3,5,7
    
    if(response == "PH0"){
        axisLimitSwitches[axis1] = false;
        axisLimitSwitches[axis2] = false;
    }else if(response == "PH1"){
        axisLimitSwitches[axis1] = true;
        axisLimitSwitches[axis2] = false;
    }else if(response == "PH2"){
        axisLimitSwitches[axis1] = false;
        axisLimitSwitches[axis2] = true;
    }else if(response == "PH3"){
        axisLimitSwitches[axis1] = true;
        axisLimitSwitches[axis2] = true;
    }else{
        emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid response to limit status query, %2").arg(device_name).arg(response));
        return;
    }
    
    //When calibrating, keep querying the axis limit until at the limit
    uint channelAxis = (currentChannel-1)*2+currentAxis-1;//0 to 7
    if(calibrating && axisLimitSwitches[channelAxis]){
        if(limitSign < 0){//at the negative limit while calibrating
            queueSerialCommand(QString("ZEROSTEPCOUNTER_%1").arg(currentAxis));
            queueSerialCommand("QUERYERROR");
            queueSerialCommand(QString("MOVEREL_%1_%2").arg(currentAxis).arg(100));//Move 100 steps to get out of the negative limit
            queueSerialCommand("QUERYERROR");
            queueSerialCommand(QString("QUERYSTATUS_%1").arg(currentAxis));
            queueSerialCommand("QUERYERROR");
        }else{
            queueSerialCommand(QString("QUERYNUMSTEPS_%1").arg(currentAxis));
            queueSerialCommand("QUERYERROR");
        }
    }else if(calibrating){
        queueSerialCommand("QUERYLIMITSTATUS");
    }
}

//number of steps taken either since zeroing the step count or since powering on the mirror
void AgilisMirrors::responseNumSteps(QString response)
{
    bool conv_ok = response.startsWith("TP");
    QStringList parts = response.split("P");
    
    uint steps;
    if(parts.length() == 2){
        steps = parts.at(1).toUInt(&conv_ok);
    }else{
        conv_ok = false;
    }
    
    uint channelAxis = (currentChannel-1)*2+currentAxis-1;//0 to 7
    if(conv_ok && calibrating){
        stepRange[channelAxis] = steps;
    }else if(!conv_ok){
        emit device_message(QString("AGILIS MIRRORS ERROR: %1: Invalid response to number of steps query, %2").arg(device_name).arg(response));
    }
    calibrating = false;
}