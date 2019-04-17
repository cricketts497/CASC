#include "include/RemoteDevice.h"

RemoteDevice::RemoteDevice(QString deviceName, CascConfig * config, QObject * parent) :
CascDevice(deviceName, config, parent),
socket(new QTcpSocket(this)),
remoteCommand(QString(""))
{
	if(device_failed)
		return;
	
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));

	connect(socket, SIGNAL(connected()), this, SLOT(writeCommand()));
	
    connect(socket, SIGNAL(readyRead()), this, SLOT(readResponse()));

	connect(connection_timer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));
	connect(socket, SIGNAL(disconnected()), connection_timer, SLOT(stop()));
    
	//send the remote command to start the device
	QString outString;
	QTextStream out(&outString);
	out << "start_" << deviceName;
	remoteCommand = out.readAll();
	
    socket->connectToHost(hostAddress, hostListenPort);
    connection_timer->start();
}

void RemoteDevice::stop_device()
{
	if(device_failed){
		emit stopped();
		return;
	}

	//send the remote command to stop the device
	QString outString;
	QTextStream out(&outString);
	out << "stop_" << device_name;
	remoteCommand = out.readAll();

    socket->connectToHost(hostAddress, hostListenPort);
    connection_timer->start();
	
	connect(socket, &QTcpSocket::disconnected, this, &CascDevice::stop_device);
}

void RemoteDevice::writeCommand()
{    
    if(remoteCommand.split("_").first() != QString("stop") && remoteCommand.split("_").first() != QString("start")){
        remoteCommand = remoteDeviceCommandQueue.dequeue();
    }
    
    socket->write(remoteCommand.toUtf8());
}


//write device commands from the widgets
void RemoteDevice::deviceCommand(QString device_com)
{
    remoteDeviceCommandQueue.enqueue(device_com);
    
    if(socket->state() == QAbstractSocket::UnconnectedState){
        socket->connectToHost(hostAddress, hostDevicePort);
        connection_timer->start();
    }
}


void RemoteDevice::readResponse()
{
    QByteArray resp = socket->readAll();
    
    if(resp.endsWith(failMessage)){
        emit device_message(QString("REMOTE %1 ERROR: fail message received from local").arg(device_name));
		emit device_fail();
        return;
    }
    
    emit newResponse(resp);
    
    //next pending command
    if(!remoteDeviceCommandQueue.isEmpty()){
        writeCommand();
    }else{
        socket->disconnectFromHost();
    }    
}


//error handling
/////////////////////////////////////////////////////////////////
void RemoteDevice::connectionTimeout()
{
	storeMessage(QString("REMOTE %1 ERROR: Connection timeout").arg(device_name), true);
	emit device_message(QString("REMOTE %1 ERROR: Connection timeout").arg(device_name));
	emit device_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->disconnectFromHost();
}

//including if remote host closes the connection
void RemoteDevice::socketError()
{	
    //listener closes the connection, ignore this
    if((remoteCommand.split("_").first() == QString("stop") || remoteCommand.split("_").first() == QString("start")) && socket->error() == QAbstractSocket::RemoteHostClosedError){ 
        return;
    }
    
	storeMessage(QString("REMOTE %1 ERROR: %2").arg(device_name).arg(socket->errorString()), true);
	emit device_message(QString("REMOTE %1 ERROR: %2").arg(device_name).arg(socket->errorString()));
	emit device_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->disconnectFromHost();
}




