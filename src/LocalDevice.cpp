#include "include/LocalDevice.h"

LocalDevice::LocalDevice(QString deviceName, CascConfig * config, QObject * parent) :
CascDevice(deviceName, config, parent),
deviceServer(new QTcpServer(this))
{
	if(device_failed)
		return;

	//start the device server
	if(!deviceServer->listen(QHostAddress::Any, hostDevicePort)){
		storeMessage(QString("LOCAL %1 ERROR: deviceServer->listen()").arg(deviceName), true);
		return;
	}
	connect(deviceServer, SIGNAL(newConnection()), this, SLOT(newCon()));
	
	connect(connection_timer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));

	storeMessage(QString("Local %1: Running, port: %2").arg(deviceName).arg(deviceServer->serverPort()), false);
}

void LocalDevice::stop_device()
{
	emit stopped();
}

void LocalDevice::newCon()
{
	socket = deviceServer->nextPendingConnection();
	deviceServer->pauseAccepting();//one connection at a time
	connect(socket, SIGNAL(disconnected()), this, SLOT(messageReceived()));
	connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(receiveCommand()));
	
	connect(socket, SIGNAL(readyRead()), connection_timer, SLOT(stop()));
	connection_timer->start();
}

void LocalDevice::receiveCommand()
{	
	QByteArray com = socket->readAll();
	QString command = QString::fromUtf8(com);

	// emit device_message(QString("Local %1: received command: %2").arg(device_name).arg(command));
	
	//connect the devices to this signal to do somthing with the command//////
	emit newCommand(command);
}

void LocalDevice::messageReceived()
{
	deviceServer->resumeAccepting();
}

//error handling
///////////////////////////////////////////////////////////////
void LocalDevice::connectionTimeout()
{
	storeMessage(QString("LOCAL %1 ERROR: Connection timeout").arg(device_name), true);
	emit device_message(QString("LOCAL %1 ERROR: Connection timeout").arg(device_name));
	emit device_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->disconnectFromHost();
}

void LocalDevice::socketError()
{
	//ignore if the remote host closes the connection
	if(socket->error() == QAbstractSocket::RemoteHostClosedError)
		return;
	
	storeMessage(QString("LOCAL %1 ERROR: %2").arg(device_name).arg(socket->errorString()), true);
	emit device_message(QString("LOCAL %1 ERROR: %2").arg(device_name).arg(socket->errorString()));
	emit device_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->disconnectFromHost();
	
}