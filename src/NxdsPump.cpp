#include "include/NxdsPump.h"

NxdsPump::NxdsPump(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(file_path, file_mutex, deviceName, config, parent),
activeQuery(QString("NONE")),
temperatureTimer(new QTimer(this)),
temperatureTimeout(1000),
speedStatusTimer(new QTimer(this)),
speedStatusTimeout(1010),
pumpStatus(QString("0")),
pumpControllerTemperature(QString("0")),
pumpServiceStatus(QString("0"))
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
    
    //setup the query timers
    temperatureTimer->setInterval(temperatureTimeout);
    connect(temperatureTimer, SIGNAL(timeout()), this, SLOT(queryTemperature()));
    connect(this, SIGNAL(device_fail()), temperatureTimer, SLOT(stop()));
    
    speedStatusTimer->setInterval(speedStatusTimeout);
    connect(speedStatusTimer, SIGNAL(timeout()), this, SLOT(querySpeedStatus()));
    connect(this, SIGNAL(device_fail()), speedStatusTimer, SLOT(stop()));
    
    if(!openSerialPort())
        return;
    
    queueSerialCommand(QString("PUMPTYPE"));
    queueSerialCommand(QString("PUMPSERVICESTATUS"));
    
    //uncomment and recompile to reset the tip seal service indicator
    //////////////////////////////////////////////////
    // queueSerialCommand(QString("TIPSEALRESET"));
    /////////////////////////////////////////////////
    
    temperatureTimer->start();
    speedStatusTimer->start();
    
    //device_name, speedStatus, controller temperature, service status
    deviceStatus = QString("Status_%1_0_0_0").arg(device_name);
}

void NxdsPump::queryTemperature()
{
    queueSerialCommand(QString("PUMPTEMPERATURE"));    
}

void NxdsPump::querySpeedStatus()
{
    queueSerialCommand(QString("PUMPSPEEDSTATUS"));    
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
    }else if(command_list.first() == QString("PUMPSERVICESTATUS")){
        toQuery = QString("?V826\r");
    }else if(command_list.first() == QString("PUMPTEMPERATURE")){
        toQuery = QString("?V808\r");
    }else if(command_list.first() == QString("PUMPSPEEDSTATUS")){
        toQuery = QString("?V802\r");
    }else if(command_list.first() == QString("TIPSEALRESET")){
        toQuery = QString("!C814 1\r");
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
    //no response was received before the timeout, do nothing
    if(response == noResponseMessage){
        activeQuery = QString("NONE");
        return;
    }
    
    // emit device_message(QString("Local NxdsPump: %1: Response: %2").arg(device_name).arg(response));
    
    if(activeQuery == QString("NONE")){
        return;
    }
    
    //check the length
    if(response.length() < 6){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Pump response %2 is invalid for query %3").arg(device_name).arg(response).arg(activeQuery));
        emit device_fail();
        activeQuery = QString("NONE");
        return;
    }
    
    //numbers in the query should be same as response
    QStringRef subResp(&response, 2,3);
    QStringRef subQuery(&activeQuery, 2,3);
    if(subResp != subQuery){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Pump response %2 is invalid for query %3").arg(device_name).arg(response).arg(activeQuery));
        emit device_fail();
        activeQuery = QString("NONE");
        return;
    }
    
    //all characters from the 6th slot and after, removing the /cr at the end
    QString outResponse = response.mid(6);
    outResponse.remove(outResponse.length()-1,1);
    
    //send to functions
    if(activeQuery == QString("?S801\r")){
        responsePumpType(outResponse);
    }else if(activeQuery == QString("?V826\r")){
        responsePumpServiceStatus(outResponse);
    }else if(activeQuery == QString("?V808\r")){
        responsePumpTemperature(outResponse);
    }else if(activeQuery == QString("?V802\r")){
        responsePumpSpeedStatus(outResponse);
    }else if(activeQuery == QString("!C814 1\r")){
        responsePumpResetTipSeal(outResponse);
    }else{
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Unknown activeQuery").arg(device_name));
        emit device_fail();
    }
       
    deviceStatus = QString("Status_%1_%2_%3_%4").arg(device_name).arg(pumpStatus).arg(pumpControllerTemperature).arg(pumpServiceStatus);
    // emit device_message(QString("Local NxdsPump: %1").arg(deviceStatus));
    
    activeQuery = QString("NONE");
}


//pump responses
////////////////////////////////////////////////////////////////////////////////////////////////
void NxdsPump::responsePumpType(QString response)
{
    QStringList type_list = response.split(";");
    
    //check for valid pump type response
    if(type_list.length() != 3){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Pump response %2 is invalid for PUMPTYPE query").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    storeMessage(QString("Local NxdsPump: %4: Pump type: %1, Software: %2, Nominal frequency: %3").arg(type_list[0]).arg(type_list[1]).arg(type_list[2]).arg(device_name), false);
    emit device_message(QString("Local NxdsPump: %4: Pump type: %1, Software: %2, Nominal frequency: %3").arg(type_list[0]).arg(type_list[1]).arg(type_list[2]).arg(device_name));

    if(type_list[0] != QString("nXDS")){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Response pump type is invalid, should be nXDS").arg(device_name));
        emit device_fail();
    }
}

void NxdsPump::responsePumpServiceStatus(QString response)
{
    QStringList service_status_list = response.split(";");
    
    //check for valid service status response
    if(service_status_list.length() != 1){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Pump response %2 is invalid for PUMPSERVICESTATUS query").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    bool conv_ok;
    int serviceStatus = service_status_list.at(0).toInt(&conv_ok, 16);
    if(!conv_ok){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: service status is invalid: %2").arg(device_name).arg(service_status_list[0]));
        emit device_fail();
        return;
    }
    
    pumpServiceStatus = service_status_list.at(0);
    
    //General service message, comes with the other service messages
    if((serviceStatus&0x0080)==0x0080){
        emit device_message(QString("Local NxdsPump: %1: Service is due, part to be serviced should be given below").arg(device_name));
    }   
    //tip seal service
    if((serviceStatus&0x0001)==0x0001){
        emit device_message(QString("Local NxdsPump: %1: Tip seal service is due").arg(device_name));
    }
    //bearing service
    if((serviceStatus&0x0002)==0x0002){
        emit device_message(QString("Local NxdsPump: %1: Bearing service is due").arg(device_name));
    }
    //controller service
    if((serviceStatus&0x0008)==0x0008){
        emit device_message(QString("Local NxdsPump: %1: Controller service is due").arg(device_name));
    }    
}

void NxdsPump::responsePumpTemperature(QString response)
{
    QStringList temperature_list = response.split(";");
    
    //check for valid pump temperature response
    if(temperature_list.length() != 2){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Pump response %2 is invalid for PUMPTEMPERATURE query").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    //check the temperature is a valid integer
    bool conv_ok;
    int temp = temperature_list.at(1).toInt(&conv_ok);
    if(!conv_ok){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Pump response %2 is invalid for PUMPTEMPERATURE query").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    //return value of -200 for either indicates temperature not utilised
    //getting -200 for pump temperature
    // emit device_message(QString("Local NxdsPump: %1: Controller temperature: %2 degrees").arg(device_name).arg(temperature_list[1]));
    pumpControllerTemperature = temperature_list[1];
}

void NxdsPump::responsePumpSpeedStatus(QString response)
{
    QStringList speed_status_list = response.split(";");
    
    //check for valid speed status response
    if(speed_status_list.length() != 5){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Pump response %2 is invalid for PUMPSPEEDSTATUS query").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    //check each register and the rotational speed with conversion
    bool conv_ok;
    int rot_speed = speed_status_list.at(0).toInt(&conv_ok);
    if(!conv_ok){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: rotational speed is invalid: %2").arg(device_name).arg(speed_status_list[0]));
        emit device_fail();
        return;
    }    
    int register1 = speed_status_list.at(1).toInt(&conv_ok, 16);
    if(!conv_ok){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: status register 1 is invalid: %2").arg(device_name).arg(speed_status_list[1]));
        emit device_fail();
        return;
    }
    int register2 = speed_status_list.at(2).toInt(&conv_ok, 16);
    if(!conv_ok){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: status register 2 is invalid: %2").arg(device_name).arg(speed_status_list[2]));
        emit device_fail();
        return;
    }
    int warning_register = speed_status_list.at(3).toInt(&conv_ok, 16);
    if(!conv_ok){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: status warning register is invalid: %2").arg(device_name).arg(speed_status_list[3]));
        emit device_fail();
        return;
    }
    int fault_register = speed_status_list.at(4).toInt(&conv_ok, 16);
    if(!conv_ok){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: status fault register is invalid: %2").arg(device_name).arg(speed_status_list[4]));
        emit device_fail();
        return;
    }
    
    // emit device_message(QString("Local NxdsPump: %1: Rotational speed: %2 Hz, register 1: %3, register 2: %4, warning register: %5, fault register: %6").arg(device_name).arg(speed_status_list[0]).arg(speed_status_list[1]).arg(speed_status_list[2]).arg(speed_status_list[3]).arg(speed_status_list[4]));
    
    pumpStatus = response;
    
    //first register messages
    //decel
    if((register1&0x0001)==0x0001){
        emit device_message(QString("Local NxdsPump: %1: Decelerating").arg(device_name));
    }
    //accel or running
    // if((register1&0x0002)==0x0002){
        // emit device_message(QString("Local NxdsPump: %1: Running").arg(device_name));
    // }
    //Standby
    if((register1&0x0004)==0x0004){
        emit device_message(QString("Local NxdsPump: %1: Standby mode").arg(device_name));
    }
    //Normal speed
    // if((register1&0x0008)==0x0008){
        // emit device_message(QString("Local NxdsPump: %1: Normal speed").arg(device_name));
    // }
    //Above ramp speed
    // if((register1&0x0010)==0x0010){
        // emit device_message(QString("Local NxdsPump: %1: Above ramp speed").arg(device_name));
    // }
    //above overload speed
    // if((register1&0x0020)==0x0020){
        // emit device_message(QString("Local NxdsPump: %1: Above overload speed").arg(device_name));
    // }
    //control mode
    //bits 6 and 7 indicate mode: 00 => none, 01 => serial, 10 => parallel, 11 => manual
    // if((register1&0x0040)==0x0040 && (~register1&0x0080)==0x0080){
        // emit device_message(QString("Local NxdsPump: %1: Serial control mode").arg(device_name));
    // }
    // if((~register1&0x0040)==0x0040 && (register1&0x0080)==0x0080){
        // emit device_message(QString("Local NxdsPump: %1: Parallel control mode").arg(device_name));
    // }
    // if((register1&0x0040)==0x0040 && (register1&0x0080)==0x0080){
        // emit device_message(QString("Local NxdsPump: %1: Manual control mode").arg(device_name));
    // }
    //Serial enable, error if not enabled
    // if((register1&0x0400)==0x0400){
    if((register1&0x0400)!=0x0400){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: Serial mode is not enabled in pump status register 1").arg(device_name));
        emit device_fail();
    }
    
    
    //second register messages
    //warning, see warning register
    // if((register2&0x0040)==0x0040){
        // emit device_message(QString("Local NxdsPump: %1: Warning status").arg(device_name));
    // }
    //alarm, see fault register
    // if((register2&0x0080)==0x0080){
        // emit device_message(QString("Local NxdsPump: %1: Fault status").arg(device_name));
    // }
    
    
    //warning register
    //Low pump controller temperature
    if((warning_register&0x0002)==0x0002){
        emit device_message(QString("Local NxdsPump: %1: WARNING: Low pump-controller temperature: below the measurable range").arg(device_name));
    }
    //pump controller regulator active
    if((warning_register&0x0040)==0x0040){
        emit device_message(QString("Local NxdsPump: %1: WARNING: Pump-controller regulator active: output current is limited due to high temperature").arg(device_name));
    }
    //high pump controller temperature
    if((warning_register&0x0400)==0x0400){
        emit device_message(QString("Local NxdsPump: %1: WARNING: High pump-controller temperature: above the measurable range").arg(device_name));
    }
    
    
    //fault register
    //over voltage trip
    if((fault_register&0x0002)==0x0002){
        emit device_message(QString("Local NxdsPump: %1: FAULT: Over voltage trip: excessive link voltage").arg(device_name));
    }
    //over current trip
    if((fault_register&0x0004)==0x0004){
        emit device_message(QString("Local NxdsPump: %1: FAULT: Over current trip: excessive motor current").arg(device_name));
    }
    //over temperature trip
    if((fault_register&0x0008)==0x0008){
        emit device_message(QString("Local NxdsPump: %1: FAULT: Over temperature trip: excessive controller temperature").arg(device_name));
    }
    //under temperature trip
    if((fault_register&0x0010)==0x0010){
        emit device_message(QString("Local NxdsPump: %1: FAULT: Under temperature trip: temperature sensor failure").arg(device_name));
    }
    //Power stage fault
    if((fault_register&0x0020)==0x0020){
        emit device_message(QString("Local NxdsPump: %1: FAULT: Power stage fault: power stage has failed").arg(device_name));
    }
    //hardware fault latch
    if((fault_register&0x0100)==0x0100){
        emit device_message(QString("Local NxdsPump: %1: FAULT: Hardware latch set: fault latch activated").arg(device_name));
    }
    //Serial control mode interlock
    if((fault_register&0x2000)==0x2000){
        emit device_message(QString("Local NxdsPump: %1: FAULT: Serial control mode interlock: serial enable went inactive when operating from serial start command").arg(device_name));
    }
    //Overload timeout
    if((fault_register&0x4000)==0x4000){
        emit device_message(QString("Local NxdsPump: %1: FAULT: Overload timeout: output frequency fell below threshold for more than allowable time").arg(device_name));
    }
    //acceleration timeout
    if((fault_register&0x8000)==0x8000){
        emit device_message(QString("Local NxdsPump: %1: FAULT: Acceleration timeout: output frequency didnt reach threshold in allowable time").arg(device_name));
    }
}

void NxdsPump::responsePumpResetTipSeal(QString response)
{
    bool conv_ok;
    int return_code = response.toInt(&conv_ok);
    if(!conv_ok){
        emit device_message(QString("LOCAL NXDSPUMP ERROR: %1: reset tip seal timer response is invalid").arg(device_name).arg(response));
        emit device_fail();
        return;
    }
    
    if(return_code == 0){
        emit device_message(QString("Local NxdsPump: %1: Successful reset of tip seal service timer").arg(device_name));
    }else{
        emit device_message(QString("Local NxdsPump: %1: FAULT: reset tip seal service timer command returned an error code: %2").arg(device_name).arg(return_code));
    }
}
