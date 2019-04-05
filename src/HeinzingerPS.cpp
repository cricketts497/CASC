#include "include/HeinzingerPS.h"

/*
Data file has format:
timestamp (qint64), applied voltage (quint64)
*/

HeinzingerPS::HeinzingerPS(uint voltage_limit, uint current_limit, QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(file_path, file_mutex, deviceName, config, parent),
time(new QDateTime()),
voltage_query_timer(new QTimer(this)),
voltage_query_timeout(5000),
activeQuery(0),
voltage_limit(voltage_limit),
current_limit(current_limit),
output_setpoint(false),
activeSetFunction(0),
queryAfterSetTimer(new QTimer(this)),
queryAfterSetTimeout(500),
voltage_setpoint(0),
current_setpoint(0),
averages_setpoint(0),
volts_ok(false),
amps_ok(false),
voltage_set(0),
current_set(0),
voltage_applied(0),
current_applied(0),
averages_set(0)
{
    if(device_failed)
        return;
    
    //start a new voltage file
	QMutexLocker file_locker(file_mutex);
	if(!data_file->open(QIODevice::WriteOnly)){
		storeMessage(QString("LOCAL SERIAL HEINZINGER ERROR: init: file->open(write)"), true);
		return;
	}
	QDataStream out(data_file);
	qint64 header = time->currentMSecsSinceEpoch();
	out << header;
	data_file->close();
    
    //set the interval to query the true voltage
	voltage_query_timer->setInterval(voltage_query_timeout);
	connect(voltage_query_timer, SIGNAL(timeout()), this, SLOT(queryAppliedVoltage()));

	//settings for serial communication with the power supplies taken from the manual
	setBaudRate(9600);
    setDataBits(8);
	setParity(0);//No parity
	setStopBits(1);
	setFlowControl(0);//SoftwareControl
    
    //commands from local and remote devices
    connect(this, SIGNAL(newCommand(QString)), this, SLOT(heinzingerCommand(QString)));
    
    //response from device
    connect(this, SIGNAL(newResponse(QString)), this, SLOT(dealWithResponse(QString)));
    
    //disable the output on fail
    connect(this, SIGNAL(device_fail()), this, SLOT(stop_device()));
    
    //allow time after setting value before query
    queryAfterSetTimer->setInterval(queryAfterSetTimeout);
    queryAfterSetTimer->setSingleShot(true);
    connect(queryAfterSetTimer, SIGNAL(timeout()), this, SLOT(queryAfterSet()));
	
	if(!openSerialPort())
        return;
    
    //reset the power supply
    writeCommand(QString("*RST \n"));
    
}

//Set the voltage to zero and turn off the output
//stopping device or failure mode
void HeinzingerPS::stop_device()
{
    setVoltage(0);
    
    if(voltage_set == 0){
        //output already disabled
        emit device_message(QString("Local Heinzinger: Output disabled"));
        SerialDevice::stop_device();
        return;
    }else if(voltage_setpoint != 0){
        emit device_message(QString("LOCAL HEINZINGER ERROR: unable to disable output"));
        emit device_fail();
    }
    //when the response comes that the output is disabled, close the serial port
    connect(this, &HeinzingerPS::voltage_set_zero, this, &SerialDevice::stop_device); 
}

//receive commands from remote device
void HeinzingerPS::heinzingerCommand(QString command)
{	
	QStringList command_list = command.split("_");
	
    bool conv_ok;
    if(command_list.first() == QString("VOLT")){
        uint voltage = command_list.at(1).toUInt(&conv_ok);
        if(!conv_ok){
            emit device_message(QString("Local Heinzinger: command: Bad voltage"));
            return;
        }
        setVoltage(voltage);
    }else if(command_list.first() == QString("CURR")){
        uint current = command_list.at(1).toUInt(&conv_ok);
        if(!conv_ok){
            emit device_message(QString("Local Heinzinger: command: Bad current"));
            return;
        }
        setCurrent(current);
    //use 0=off or 1=on for remote on/off commands
    }else if(command_list.first() == QString("OUTP")){
        uint on = command_list.at(1).toUInt(&conv_ok);
        if(!conv_ok){
            emit device_message(QString("Local Heinzinger: command: Bad on/off, use 0=off, 1=on"));
            return;
        }
        if(on == 0){
            setOutput(false);
        }else if(on == 1){
            setOutput(true);
        }
    }
}

//query values after setting them with some delay
void HeinzingerPS::queryAfterSet()
{
    switch(activeSetFunction){
        case 0: return;
            break;
        case 1: querySetVoltage();
            break;
        case 2: querySetCurrent();
            break;
        case 3: queryAverages();
            break;
            
        default: {
            emit device_message(QString("LOCAL HEINZINGER ERROR: Unknown activeSetFunction"));
            emit device_fail();
        }
    }    
}


//send serial responses to correct function
void HeinzingerPS::dealWithResponse(QString response)
{
    emit device_message(QString("Local Heinzinger: Response: %1").arg(response));
    switch(activeQuery) {
        case 0: return;
        case 1: responseID(response);
            break;
        case 2: responseVersion(response);
            break;
        case 3: responseSetVoltage(response);
            break;
        case 4: responseSetCurrent(response);
            break;
        case 5: responseAppliedVoltage(response);
            break;
        case 6: responseAppliedCurrent(response);
            break;
        case 7: responseAverages(response);
            break;
        
        default: {
            emit device_message(QString("LOCAL HEINZINGER ERROR: Unknown activeQuery"));
            emit device_fail();
        }
    }
        
    activeQuery = 0;
}

//serial commands
/////////////////////////////////////////////////////
void HeinzingerPS::setOutput(bool on)
{
    if(!volts_ok || !amps_ok || !on){
        if(writeCommand(QString("OUTP OFF\n"))){
            output_setpoint = false;
            
            if(voltage_query_timer->isActive())
                voltage_query_timer->stop();
        }
    }else{
        if(writeCommand(QString("OUTP ON\n"))){
            output_setpoint = true;
            
            //start querying the applied voltage
            voltage_query_timer->start();
        }
    }        
}

void HeinzingerPS::setVoltage(uint voltage)
{
    volts_ok = false;
    if(voltage > voltage_limit){
        emit device_message(QString("Local Heinzinger: Trying to set voltage %1 V, above maximum value %2 V").arg(voltage).arg(voltage_limit));
        return;        
    }
    
    QString outString;
	QTextStream out(&outString);
	out << "VOLT " << voltage << "\n";
    if(writeCommand(out.readAll())){
        voltage_setpoint = voltage;
        activeSetFunction = 1;
        queryAfterSetTimer->start();
    }
}

void HeinzingerPS::setCurrent(uint current)
{
    amps_ok = false;
    if(current > current_limit){
        emit device_message(QString("Local Heinzinger: Trying to set current %1 mA, above maximum value %2 mA").arg(current).arg(current_limit));
        return;        
    }
    
    QString outString;
	QTextStream out(&outString);
	out << "CURR " << current << "\n";
    if(writeCommand(out.readAll())){
        current_setpoint = current;
        activeSetFunction = 2;
        queryAfterSetTimer->start();
    }
}

void HeinzingerPS::setAverages(uint averages)
{
    if(!possible_averages.contains(averages)){
        emit device_message(QString("Local Heinzinger: Unable to set averages to %1").arg(averages));
        return;        
    }
    
    QString outString;
	QTextStream out(&outString);
	out << "AVER " << averages << "\n";
    if(writeCommand(out.readAll())){
        averages_setpoint = averages;
        activeSetFunction = 3;
        queryAfterSetTimer->start();
    }
}


//serial queries
////////////////////////////////////////////////////
void HeinzingerPS::queryID()
{
    if(activeQuery != 0){
        emit device_message(QString("Local Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("*IDN? \n"), true))
        activeQuery = 1;
}

void HeinzingerPS::queryVersion()
{
    if(activeQuery != 0){
        emit device_message(QString("Local Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("VERS? \n"), true))
        activeQuery = 2;
}

//voltage setting
void HeinzingerPS::querySetVoltage()
{
    if(activeQuery != 0){
        emit device_message(QString("Local Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("VOLT? \n"), true))
        activeQuery = 3;
}

//current limit set
void HeinzingerPS::querySetCurrent()
{
    if(activeQuery != 0){
        emit device_message(QString("Local Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("CURR? \n"), true))
        activeQuery = 4;
}

void HeinzingerPS::queryAppliedVoltage()
{
    if(activeQuery != 0){
        emit device_message(QString("Local Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("MEAS:VOLT? \n"), true))
        activeQuery = 5;
}

void HeinzingerPS::queryAppliedCurrent()
{
    if(activeQuery != 0){
        emit device_message(QString("Local Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("MEAS:CURR? \n"), true))
        activeQuery = 6;
}

void HeinzingerPS::queryAverages()
{
    if(activeQuery != 0){
        emit device_message(QString("Local Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("AVER? \n"), true))
        activeQuery = 7;
}


//serial responses
////////////////////////////////////////////////////
void HeinzingerPS::responseID(QString response)
{
    emit device_message(QString("Local Heinzinger: Serial number %1").arg(response));
}

void HeinzingerPS::responseVersion(QString response)
{
    emit device_message(QString("Local Heinzinger: Version number %1").arg(response));
}

void HeinzingerPS::responseSetVoltage(QString response)
{
    //If response contains a decimal point
    QStringList response_list = response.split(".");
    
    bool response_status = true;
    uint response_voltage;
    if(response_list.first() == QString("")){
        response_voltage = 0;
    }else{
        response_voltage = response_list.first().toUInt(&response_status);
    }
    
    if(!response_status){
        volts_ok = false;
		emit device_message(QString("LOCAL HEINZINGER ERROR: Set voltage response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
    
    voltage_set = response_voltage;
    
    if(voltage_set != voltage_setpoint){
        volts_ok = false;
        emit device_message(QString("LOCAL HEINZINGER ERROR: voltage set query response (%1) not equal to voltage setpoint (%2)").arg(voltage_set).arg(voltage_setpoint));
		emit device_fail();
    }else{
        volts_ok = true;
    }
    
    if(voltage_set == 0)
        emit voltage_set_zero();
}

void HeinzingerPS::responseSetCurrent(QString response)
{
    //If response contains a decimal point
    QStringList response_list = response.split(".");
    
    bool response_status = true;
	uint response_current;
    if(response_list.first() == QString("")){
        response_current = 0;
    }else{
        response_current = response_list.first().toUInt(&response_status);
    }
    
    if(!response_status){
        amps_ok = false;
		emit device_message(QString("LOCAL HEINZINGER ERROR: Set current response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
    
    current_set = response_current;
    
    if(current_set != current_setpoint){
        amps_ok = false;
        emit device_message(QString("LOCAL HEINZINGER ERROR: current set query response (%1) not equal to current setpoint (%2)").arg(current_set).arg(current_setpoint));
		emit device_fail();
    }else{
        amps_ok = true;
    }
}

void HeinzingerPS::responseAppliedVoltage(QString response)
{
    QStringList response_list = response.split(".");

    bool response_status = true;
	uint response_voltage;
    if(response_list.first() == QString("")){
        response_voltage = 0;
    }else{
        response_voltage = response_list.first().toUInt(&response_status);
    }
    
    //Convert the number after the decimal to a separate integer
    uint response_decimal = 0;
    if(response_list.length() > 1)
        response_decimal = response_list.at(1).toUInt(&response_status);
    
    if(!response_status){
		emit device_message(QString("LOCAL HEINZINGER ERROR: Applied voltage response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
    
    voltage_applied = response_voltage;
    voltage_decimal_applied = response_decimal;
    
    //write the voltage to file witha timestamp
	QMutexLocker file_locker(file_mutex);
	if(!data_file->open(QIODevice::Append)){
		emit device_message(QString("LOCAL HEINZINGER ERROR: voltage_file->open(append)"));
		emit device_fail();
		return;
	}
	QDataStream out(data_file);
    
    qint64 timestamp = time->currentMSecsSinceEpoch();
    out << timestamp << voltage_applied << voltage_decimal_applied;
    
    data_file->close();
}

void HeinzingerPS::responseAppliedCurrent(QString response)
{
    //If response contains a decimal point
    QStringList response_list = response.split(".");
    
    bool response_status = true;
	uint response_current;
    if(response_list.first() == QString("")){
        response_current = 0;
    }else{
        response_current = response_list.first().toUInt(&response_status);
    }

    if(!response_status){
		emit device_message(QString("LOCAL HEINZINGER ERROR: Applied voltage response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
    
    current_applied = response_current;
}

void HeinzingerPS::responseAverages(QString response)
{
    bool response_status;
	uint response_averages = response.toUInt(&response_status);
    if(!response_status){
		emit device_message(QString("LOCAL HEINZINGER ERROR: Averages response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
    
    averages_set = response_averages;
    
    if(averages_set != averages_setpoint){
        emit device_message(QString("LOCAL HEINZINGER ERROR: averages query response (%1) not equal to averages setpoint (%2)").arg(averages_set).arg(averages_setpoint));
		emit device_fail();
    }
}
















