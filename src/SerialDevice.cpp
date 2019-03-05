#include "include/QCDevice.h"

SerialDevice::SerialDevice(QString deviceName, CascConfig * config, QObject * parent) :
LocalDevice(deviceName, config, parent),
serial_port(new QSerialPort(this)),
serial_timer(new QTimer(this)),
serial_timeout(1000)
{
	serial_timer->setSingleShot(true);
	serial_timer->setInterval(timeout);
	
	connect(serial_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialError()));
	connect(serial_port, SIGNAL(readyRead()), this, SLOT(readResponse()));
	
	connect(serial_timer, SIGNAL(timeout()), this, SLOT(serialTimeout()));
}

void SerialDevice::stop_device()
{
	if(serial_port->isOpen()){
		connect(serial_port, &QSerialPort::aboutToClose, this, &LocalDevice::stop_device);
		serial_port->close();
	}else{
		emit stopped();
	}
}

// void SerialDevice::writeCommand(QString command)
// {
	// serial_port->write(command.toUtf8());
// }

void SerialPort::readResponse()
{
	QByteArray resp = serial_port->readAll();
	QString response = QString::fromUtf8(resp);
	
	emit newResponse(response);
}


//error handling
/////////////////////////////////////////////////////////////
void SerialDevice::serialTimeout()
{
	StoreMessage(QString("LOCAL SERIAL %1 ERROR: Serial connection timeout").arg(device_name), true);
	emit device_message(QString("LOCAL SERIAL %1 ERROR: Serial connection timeout").arg(device_name));
	emit device_fail();
	
	if(serial_port->isOpen())
		serial_port->close();
}


void SerialDevice::serialError()
{
	StoreMessage(QString("LOCAL SERIAL %1 ERROR: %2").arg(device_name).arg(serial_port->errorString()), true);
	emit device_message(QString("LOCAL SERIAL %1 ERROR: %2").arg(device_name).arg(serial_port->errorString()));
	emit device_fail();
	
	if(serial_port->isOpen())
		serial_port->close();
}