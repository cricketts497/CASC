#include "include/SerialDevice.h"

SerialDevice::SerialDevice(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
LocalDataDevice(file_path, file_mutex, deviceName, config, parent),
serial_timeout(2000),
serial_response_wait(300),
serial_port(new QSerialPort(this)),
serial_timer(new QTimer(this)),
commandInProgress(false)
{
	if(device_failed)
		return;
	
    connect(this, &LocalDevice::newLocalCommand, this, &SerialDevice::queueSerialCommand);
    
    //setup the timeout timer for the serial communication
	serial_timer->setSingleShot(true);
	serial_timer->setInterval(serial_timeout);
	
    //error handling
	connect(serial_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialError()));
	connect(serial_timer, SIGNAL(timeout()), this, SLOT(serialTimeout()));
    
    // connect(this, SIGNAL(device_fail()), this, SLOT(stop_device()));
    
    //response handling
	connect(serial_port, SIGNAL(readyRead()), serial_timer, SLOT(stop()));
    connect(serial_port, SIGNAL(readyRead()), this, SLOT(readResponse()));
	
	//get the serial port name from the config file, 4th argument in line
	QStringList device = config->getDevice(deviceName);
	if(device.size() < 4){
		storeMessage(QString("LOCAL SERIAL ERROR: %1: Device not found in config").arg(deviceName), true);
		return;
	}
	serial_port->setPortName(device.at(3));
}

bool SerialDevice::openSerialPort()
{
    bool open = true;
    if(!serial_port->isOpen())
        open = serial_port->open(QIODevice::ReadWrite);
    return open;
}

void SerialDevice::stop_device()
{
    //Don't delete the device until the serial connection is closed
	if(serial_port->isOpen()){
		connect(serial_port, &QSerialPort::aboutToClose, this, &LocalDevice::stop_device);
		serial_port->close();
	}else{
		emit stopped();
	}
    emit device_message(QString("Local serial: %1: Serial port closed").arg(device_name));
}

void SerialDevice::queueSerialCommand(QString command)
{
    serialCommandQueue.enqueue(command);
    
    if(!commandInProgress){
        emit serialComFinished();
    }
}

//protected settings
///////////////////////////////////////////////////////
void SerialDevice::setBaudRate(int rate)
{
    if(rate == 9600){
        serial_port->setBaudRate(QSerialPort::Baud9600);
    }else{
        emit device_message(QString("LOCAL SERIAL ERROR: %1: Invalid Baud rate").arg(device_name));
        emit device_fail();
    }
}

void SerialDevice::setDataBits(int bits)
{
    if(bits == 8){
        serial_port->setDataBits(QSerialPort::Data8);
    }else{
        emit device_message(QString("LOCAL SERIAL ERROR: %1: Invalid number of data bits").arg(device_name));
        emit device_fail();
    }
}

void SerialDevice::setParity(int parity)
{
    if(parity == 0){
        serial_port->setParity(QSerialPort::NoParity);
    }else{
        emit device_message(QString("LOCAL SERIAL ERROR: %1: Invalid parity").arg(device_name));
        emit device_fail();
    }
}

void SerialDevice::setStopBits(int bits)
{
    if(bits == 1){
        serial_port->setStopBits(QSerialPort::OneStop);
    }else{
        emit device_message(QString("LOCAL SERIAL ERROR: %1: Invalid number of stop bits").arg(device_name));
        emit device_fail();
    }
}

void SerialDevice::setFlowControl(int type)
{
    if(type == 0){
        serial_port->setFlowControl(QSerialPort::SoftwareControl);
    }else{
        emit device_message(QString("LOCAL SERIAL ERROR: %1: Invalid flow control setting").arg(device_name));
        emit device_fail();
    }
}


//read write operations
///////////////////////////////////////////////////////
bool SerialDevice::writeCommand(QString command, bool response)
{
    if(!serial_port->isOpen())
        return false;
    
    //don't write the message if already waiting for a reply from another command
    if(commandInProgress){
        emit device_message(QString("Local serial: %1: Busy waiting for reply").arg(device_name));
        return false;
    }
    
    storeMessage(QString("Local serial: %1: Writing command: %2").arg(device_name).arg(command), false);
    emit device_message(QString("Local serial: %1: Writing command: %2").arg(device_name).arg(command));
    
    QByteArray cm = command.toUtf8();
    serial_port->write(cm);
        
    expectResponse = response;
        
    commandInProgress = true;
        
    serial_timer->start();

    return true;
}

void SerialDevice::readResponse()
{
    if(!expectResponse)
        return;
    
    if(serial_timer->isActive())
        serial_timer->stop();
    
    //wait for the rest of the response
    QThread::msleep(serial_response_wait);
    
    QByteArray resp = serial_port->readAll();
    QString response = QString::fromUtf8(resp);
    
    emit newSerialResponse(response);
    commandInProgress = false;
    emit serialComFinished();
}

//error handling
/////////////////////////////////////////////////////////////
void SerialDevice::serialTimeout()
{
    if(serial_timer->isActive())
        serial_timer->stop();
    
    //either a failure so want to be able to send stop command or not expecting response
    commandInProgress = false;

    //not expecting a response
    if(!expectResponse){
        emit serialComFinished();
        return;
    }
    
	storeMessage(QString("LOCAL SERIAL ERROR: %1: Serial connection timeout").arg(device_name), true);
	emit device_message(QString("LOCAL SERIAL ERROR: %1: Serial connection timeout").arg(device_name));
	emit device_fail();
	
	if(serial_port->isOpen())
		serial_port->close();
}


void SerialDevice::serialError()
{
	if(serial_port->error() == QSerialPort::NoError){
		return;
	}else if(serial_port->error() == QSerialPort::DeviceNotFoundError){
        storeMessage(QString("LOCAL SERIAL ERROR: %1: Serial device not found").arg(device_name), true);
        emit device_message(QString("LOCAL SERIAL ERROR: %1: Serial device not found").arg(device_name));
        emit device_fail();
    }else{
        storeMessage(QString("LOCAL SERIAL ERROR: %1: %2").arg(device_name).arg(serial_port->errorString()), true);
        emit device_message(QString("LOCAL SERIAL ERROR: %1: %2").arg(device_name).arg(serial_port->errorString()));
        emit device_fail();
    }
}








