#include "include/SerialDevice.h"

SerialDevice::SerialDevice(QString deviceName, CascConfig * config, QObject * parent) :
LocalDevice(deviceName, config, parent),
serial_port(new QSerialPort(this)),
serial_timer(new QTimer(this)),
serial_timeout(10000)
{
	if(device_failed)
		return;
	
	serial_timer->setSingleShot(true);
	serial_timer->setInterval(serial_timeout);
	
	connect(serial_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialError()));
	
	connect(serial_timer, SIGNAL(timeout()), this, SLOT(serialTimeout()));
	connect(serial_port, SIGNAL(readyRead()), serial_timer, SLOT(stop()));
	
	//get the serial port name from the config file, 4th argument in line
	QStringList device = config->getDevice(deviceName);
	if(device.size() < 4){
		storeMessage(QString("LOCAL SERIAL %1 ERROR: Device not found in config").arg(deviceName), true);
		return;
	}
	serial_port->setPortName(device.at(3));
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

void SerialDevice::waitForResponse()
{
	disconnect(serial_port, SIGNAL(bytesWritten(qint64)),0,0);
	emit device_message(QString("Serial: bytesWritten"));
	connect(serial_port, SIGNAL(readyRead()), this, SLOT(readResponse()));
}

void SerialDevice::readResponse()
{
	emit device_message(QString("Serial: response"));
	disconnect(serial_port, SIGNAL(readyRead()), this, SLOT(readResponse()));
	
	QByteArray resp = serial_port->readAll();
	QString response = QString::fromUtf8(resp);
	
	emit newResponse(response);
}


//error handling
/////////////////////////////////////////////////////////////
void SerialDevice::serialTimeout()
{
	storeMessage(QString("LOCAL SERIAL %1 ERROR: Serial connection timeout").arg(device_name), true);
	emit device_message(QString("LOCAL SERIAL %1 ERROR: Serial connection timeout").arg(device_name));
	emit device_fail();
	
	if(serial_port->isOpen())
		serial_port->close();
}


void SerialDevice::serialError()
{
	if(serial_port->error() == QSerialPort::NoError)
		return;
	
	storeMessage(QString("LOCAL SERIAL %1 ERROR: %2").arg(device_name).arg(serial_port->errorString()), true);
	emit device_message(QString("LOCAL SERIAL %1 ERROR: %2").arg(device_name).arg(serial_port->errorString()));
	emit device_fail();
	
	if(serial_port->isOpen())
		serial_port->close();
}