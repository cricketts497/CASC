#include "include/HeinzingerPS.h"

HeinzingerPS::HeinzingerPS(QString deviceName, QString file_path, QMutex * file_mutex, CascConfig * config, QObject * parent) :
SerialDevice(deviceName, config, parent),
voltage_file(new QFile(file_path)),
file_mutex(file_mutex),
time(new QDateTime()),
voltage_query_timer(new QTimer(this)),
voltage_query_timeout(10000),
voltage_setpoint(0),
nAverages(4)
{
	if(device_failed)
		return;
	
	//start a new voltage file
	QMutexLocker file_locker(file_mutex);
	if(!voltage_file->open(QIODevice::WriteOnly)){
		storeMessage(QString("LOCAL SERIAL HEINZINGER ERROR: init: file->open(write)"), true);
		return;
	}
	QDataStream out(voltage_file);
	qint64 header = time->currentMSecsSinceEpoch();
	out << header;
	voltage_file->close();
	
	//set the interval to query the true voltage
	voltage_query_timer->setInterval(voltage_query_timeout);
	connect(voltage_query_timer, SIGNAL(timeout()), this, SLOT(queryVoltage()));
	
	connect(serial_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(heinzingerError()));
	connect(this, SIGNAL(device_fail()), this, SLOT(heinzingerError()));
	
	//settings for serial communication with the power supplies taken from the manual
	serial_port->setBaudRate(QSerialPort::Baud9600);
	serial_port->setDataBits(QSerialPort::Data8);
	serial_port->setParity(QSerialPort::NoParity);
	serial_port->setStopBits(QSerialPort::OneStop);
	serial_port->setFlowControl(QSerialPort::SoftwareControl);//HardwareControl or SoftwareControl	
	
	serial_port->open(QIODevice::ReadWrite);	
	// voltage_query_timer->start();
	
	//connect with remote devices
	connect(this, SIGNAL(newCommand(QString)), this, SLOT(remoteVoltageCommand(QString)));

}

/////////////////////////////////////////////////////////////

void HeinzingerPS::setVoltage(uint voltage)
{
	voltage_setpoint = voltage;
	
	//reset the power supply
	serial_port->write("*RST\n");
	connect(serial_port, SIGNAL(bytesWritten(qint64)), this, SLOT(sendSetVolt()));
}
	
void HeinzingerPS::sendSetVolt()
{
	disconnect(serial_port, SIGNAL(bytes_written(qint64)),0,0);
	//set the voltage
	QString outString;
	QTextStream out(&outString);
	out << "VOLT " << voltage_setpoint << "\n";
	emit device_message(QString("Heinzinger: sending %1").arg(out.readAll()));
	serial_port->write(out.readAll().toUtf8());
	connect(serial_port, SIGNAL(bytesWritten(qint64)), this, SLOT(sendQueryAppliedVolt()));
}

void HeinzingerPS::sendQueryAppliedVolt()
{	
	disconnect(serial_port, SIGNAL(bytes_written(qint64)),0,0);
	
	connect(serial_port, SIGNAL(bytesWritten(qint64)), this, SLOT(waitForResponse()));
	connect(this, SIGNAL(newResponse(QString)), this, SLOT(applyVoltage(QString)));
	
	//query the voltage applied
	emit device_message(QString("Heinzinger: sending VOLT?"));
	serial_port->write(QString("VOLT?\n").toUtf8());
	serial_timer->start();
}

void HeinzingerPS::bytesWrite()
{
	emit device_message(QString("Heinzinger: bytes to write %1").arg(serial_port->bytesToWrite()));
}

void HeinzingerPS::applyVoltage(QString response)
{
	disconnect(this, SIGNAL(newReponse(QString)), 0,0);
	
	emit device_message(QString("Heinzinger: received %1").arg(response));
	bool response_status;
	uint applied_voltage = response.toUInt(&response_status);
	if(!response_status){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Applied voltage response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
	
	if(applied_voltage != voltage_setpoint){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Voltage applied different to setpoint: setpoint=%1 V, applied=%2 V").arg(voltage_setpoint).arg(applied_voltage));
		emit device_fail();
		return;
	}
	
	//turn on the output
	emit device_message(QString("Heinzinger: sending OUTP ON"));
	serial_port->write("OUTP ON\n");
	
	//set the number of recordings for average value calculation
	QString outString;
	QTextStream out(&outString);
	out << "AVER " << nAverages << "\n";
	emit device_message(QString("Heinzinger: sending %1").arg(out.readAll()));
	serial_port->write(out.readAll().toUtf8());
	
	//query the number of averages
	connect(this, SIGNAL(newResponse(QString)), this, SLOT(checkAverages(QString)));
	emit device_message(QString("Heinzinger: sending AVER?"));
	serial_port->write("AVER?\n");
	serial_timer->start();	
}

void HeinzingerPS::checkAverages(QString response)
{
	emit device_message(QString("Heinzinger: received %1").arg(response));

	disconnect(this, SIGNAL(newResponse(QString)),0,0);
	
	bool response_status;
	uint applied_nAverages = response.toUInt(&response_status);
	if(!response_status){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Applied averages response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
	
	if(applied_nAverages != nAverages){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: nAverages applied different to setpoint: setpoint=%1, applied=%2").arg(nAverages).arg(applied_nAverages));
		emit device_fail();
		return;
	}
	
}

///////////////////////////////////////////////////////////////////////////////

void HeinzingerPS::queryVoltage()
{
	//Query the effective output voltage
	serial_port->write("MEAS:VOLT?\n");
	connect(this, SIGNAL(newResponse(QString)), this, SLOT(readbackVoltage(QString)));
	serial_timer->start();
}

void HeinzingerPS::readbackVoltage(QString response)
{
	qint64 current_time = time->currentMSecsSinceEpoch();
	
	disconnect(this, SIGNAL(newResponse(QString)),0,0);
	
	bool response_status;
	qreal response_true_voltage = response.toDouble(&response_status);
	if(!response_status){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: True voltage response invalid: %1").arg(response));
		emit device_fail();
		return;
	}
	true_voltage = response_true_voltage;
	
	// //save the voltage to file
	// QMutexLocker file_locker(file_mutex);
	// if(!voltage_file->open(QIODevice::Append)){
		// emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: readbackVoltage: file->open(append)"));
		// emit device_fail();
		// return;
	// }
	// QDataStream out(voltage_file);
	// out << current_time << true_voltage;
	// voltage_file->close();
	
	emit newTrueVoltage(true_voltage);
}


//handling requests from remote device
///////////////////////////////////////////////////////
void HeinzingerPS::remoteVoltageCommand(QString command)
{	
	QStringList command_list = command.split("_");
	
	if(command_list.first() != QString("voltage"))
		return;
	
	//check for send data command
	if(command_list.at(1) != QString("?")){
		QDataStream socket_stream(socket);
		socket_stream << true_voltage;
	}else{
		socket->disconnectFromHost();
		
		bool good_conversion;
		uint voltage = command_list.at(1).toUInt(&good_conversion);
		if(!good_conversion){
			emit device_message(QString("Local Serial Heinzinger: remoteVoltageCommand: Bad command"));
			return;
		}
		setVoltage(voltage);
	}
}


//error handling
///////////////////////////////////////////////////////
void HeinzingerPS::heinzingerError()
{
	if(voltage_query_timer->isActive())
		voltage_query_timer->stop();
}








