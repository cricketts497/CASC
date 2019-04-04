#include "include/HeinzingerPS.h"

/*
Data file has format:
timestamp (qint64), applied voltage (quint64)
*/

HeinzingerPS::HeinzingerPS(uint voltage_limit, uint current_limit, QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
SerialDevice(file_path, file_mutex, deviceName, config, parent),
time(new QDateTime()),
voltage_query_timer(new QTimer(this)),
voltage_query_timeout(10000),
activeQuery(0),
voltage_limit(voltage_limit),
current_limit(current_limit),
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
    voltage_query_timer->start();
    
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
	
	if(!openSerialPort())
        return;
    
    //reset the power supply
    writeCommand(QString("*RST \n"));
    
    //read 16 averages for each applied voltage output
    setAverages(16);
    
    //current limit of 1mA
    setCurrent(1);
    
    //set voltage to zero to start
    setVoltage(0);
}

//receive commands from remote device
void HeinzingerPS::heinzingerCommand(QString command)
{	
	QStringList command_list = command.split("_");
	
    bool conv_ok;
    if(command_list.first() == QString("VOLT")){
        uint voltage = command_list.at(1).toUInt(&conv_ok);
        if(!conv_ok){
            emit device_message(QString("Local Serial Heinzinger: remoteCommand: Bad voltage"));
            return;
        }
        setVoltage(voltage);
    }else if(command_list.first() == QString("CURR")){
        uint current = command_list.at(1).toUInt(&conv_ok);
        if(!conv_ok){
            emit device_message(QString("Local Serial Heinzinger: remoteCommand: Bad current"));
            return;
        }
        setCurrent(current);
    //use 0=off or 1=on for remote on/off commands
    }else if(command_list.first() == QString("OUTP")){
        uint on = command_list.at(1).toUInt(&conv_ok);
        if(!conv_ok){
            emit device_message(QString("Local Serial Heinzinger: remoteCommand: Bad on/off, use 0=off, 1=on"));
            return;
        }
        if(on == 0){
            setOutput(false);
        }else if(on == 1){
            setOutput(true);
        }
    }
}


//send serial responses to correct function
void HeinzingerPS::dealWithResponse(QString response)
{
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
            emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Unknown activeQuery"));
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
        writeCommand(QString("OUTP OFF\n"));
    }else{
        writeCommand(QString("OUTP ON\n"));
    }        
}

void HeinzingerPS::setVoltage(uint voltage)
{
    volts_ok = false;
    if(voltage > voltage_limit){
        emit device_message(QString("Local serial Heinzinger: Trying to set voltage %1 V, above maximum value %2 V").arg(voltage).arg(voltage_limit));
        return;        
    }
    
    QString outString;
	QTextStream out(&outString);
	out << "VOLT " << voltage << "\n";
    if(writeCommand(out.readAll())){
        voltage_setpoint = voltage;
        querySetVoltage();
    }
}

void HeinzingerPS::setCurrent(uint current)
{
    amps_ok = false;
    if(current > current_limit){
        emit device_message(QString("Local serial Heinzinger: Trying to set current %1 mA, above maximum value %2 mA").arg(current).arg(current_limit));
        return;        
    }
    
    QString outString;
	QTextStream out(&outString);
	out << "CURR " << current << "\n";
    if(writeCommand(out.readAll())){
        current_setpoint = current;
        querySetCurrent();
    }
}

void HeinzingerPS::setAverages(uint averages)
{
    if(!possible_averages.contains(averages)){
        emit device_message(QString("Local serial Heinzinger: Unable to set averages to %1").arg(averages));
        return;        
    }
    
    QString outString;
	QTextStream out(&outString);
	out << "AVER " << averages << "\n";
    if(writeCommand(out.readAll())){
        averages_setpoint = averages;
        queryAverages();
    }
}


//serial queries
////////////////////////////////////////////////////
void HeinzingerPS::queryID()
{
    if(activeQuery != 0){
        emit device_message(QString("Local serial Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("*IDN? \n"), true))
        activeQuery = 1;
}

void HeinzingerPS::queryVersion()
{
    if(activeQuery != 0){
        emit device_message(QString("Local serial Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("VERS? \n"), true))
        activeQuery = 2;
}

//voltage setting
void HeinzingerPS::querySetVoltage()
{
    if(activeQuery != 0){
        emit device_message(QString("Local serial Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("VOLT? \n"), true))
        activeQuery = 3;
}

//current limit set
void HeinzingerPS::querySetCurrent()
{
    if(activeQuery != 0){
        emit device_message(QString("Local serial Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("CURR? \n"), true))
        activeQuery = 4;
}

void HeinzingerPS::queryAppliedVoltage()
{
    if(activeQuery != 0){
        emit device_message(QString("Local serial Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("MEAS:VOLT? \n"), true))
        activeQuery = 5;
}

void HeinzingerPS::queryAppliedCurrent()
{
    if(activeQuery != 0){
        emit device_message(QString("Local serial Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("MEAS:CURR? \n"), true))
        activeQuery = 6;
}

void HeinzingerPS::queryAverages()
{
    if(activeQuery != 0){
        emit device_message(QString("Local serial Heinzinger: Busy waiting for reply"));
        return;
    }
    if(writeCommand(QString("AVER? \n"), true))
        activeQuery = 7;
}


//serial responses
////////////////////////////////////////////////////
void HeinzingerPS::responseID(QString response)
{
    emit device_message(QString("Local serial Heinzinger: Serial number %1").arg(response));
}

void HeinzingerPS::responseVersion(QString response)
{
    emit device_message(QString("Local serial Heinzinger: Version number %1").arg(response));
}

void HeinzingerPS::responseSetVoltage(QString response)
{
    bool response_status;
	uint response_voltage = response.toUInt(&response_status);
    if(!response_status){
        volts_ok = false;
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Set voltage response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
    
    voltage_set = response_voltage;
    
    if(voltage_set != voltage_setpoint){
        volts_ok = false;
        emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: voltage set query response (%1) not equal to voltage setpoint (%2)").arg(voltage_set).arg(voltage_setpoint));
		emit device_fail();
    }else{
        volts_ok = true;
    }
}

void HeinzingerPS::responseSetCurrent(QString response)
{
    bool response_status;
	uint response_current = response.toUInt(&response_status);
    if(!response_status){
        amps_ok = false;
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Set current response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
    
    current_set = response_current;
    
    if(current_set != current_setpoint){
        amps_ok = false;
        emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: current set query response (%1) not equal to current setpoint (%2)").arg(current_set).arg(current_setpoint));
		emit device_fail();
    }else{
        amps_ok = true;
    }
}

void HeinzingerPS::responseAppliedVoltage(QString response)
{
    bool response_status;
	uint response_voltage = response.toUInt(&response_status);
    if(!response_status){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Applied voltage response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
    
    voltage_applied = response_voltage;
    
    //write the voltage to file witha timestamp
	QMutexLocker file_locker(file_mutex);
	if(!data_file->open(QIODevice::Append)){
		emit device_message(QString("HEINZINGER ERROR: voltage_file->open(append)"));
		emit device_fail();
		return;
	}
	QDataStream out(data_file);
    
    qint64 timestamp = time->currentMSecsSinceEpoch();
    out << timestamp << voltage_applied;
    
    data_file->close();
}

void HeinzingerPS::responseAppliedCurrent(QString response)
{
    bool response_status;
	uint response_current = response.toUInt(&response_status);
    if(!response_status){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Applied voltage response invalid: %1").arg(response));
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
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Averages response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
    
    averages_set = response_averages;
    
    if(averages_set != averages_setpoint){
        emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: averages query response (%1) not equal to averages setpoint (%2)").arg(averages_set).arg(averages_setpoint));
		emit device_fail();
    }
}
















