#include "include/RemoteHeinzingerPS.h"

RemoteHeinzingerPS::RemoteHeinzingerPS(QString deviceName, CascConfig * config, QObject * parent) :
RemoteDevice(deviceName, config, parent),
voltage_query_timer(new QTimer(this)),
voltage_query_timeout(5000)
{
	connect(socket, SIGNAL(readyRead()), this, SLOT(receiveVoltage()));
	
	//set the interval to query the true voltage
	voltage_query_timer->setInterval(voltage_query_timeout);
	connect(voltage_query_timer, SIGNAL(timeout()), this, SLOT(askVoltage()));
	
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(heinzingerError()));
	connect(this, SIGNAL(device_fail()), this, SLOT(heinzingerError()));
	
	voltage_query_timer->start();
}

///////////////////////////////////////

void RemoteHeinzingerPS::set_voltage(uint voltage)
{
	//set command
	QString c_string;
	QTextStream c(&c_string);
	c << "voltage_" << voltage;
	command = c.readAll();
	
	socket->connectToHost(hostAddress, hostDevicePort);
	connection_timer->start();
}

/////////////////////////////////////

void RemoteHeinzingerPS::askVoltage()
{
	//check for other operations
	if(socket->state() != QAbstractSocket::UnconnectedState)
		return;
	
	//query command
	command = QString("voltage_?");
	
	socket->connectToHost(hostAddress, hostDevicePort);
	connection_timer->start();
}

void RemoteHeinzingerPS::receiveVoltage()
{
	QByteArray vol_bytes = socket->readAll();
	socket->disconnectFromHost();
	
	QString vol_string = QString::fromUtf8(vol_bytes);
	
	bool response_status;
	int voltage = vol_string.toInt(&response_status);
	if(!response_status){
		emit device_message(QString("REMOTE HEINZINGER ERROR: True voltage response invalid: %1").arg(vol_string));
		emit device_fail();
		return;
	}
	
	emit newTrueVoltage(voltage);	
}

//error handling
///////////////////////////////////////////////////////
void RemoteHeinzingerPS::heinzingerError()
{
	if(voltage_query_timer->isActive())
		voltage_query_timer->stop();
}
