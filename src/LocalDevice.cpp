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
	connect(deviceServer, SIGNAL(newConnection()), this, SLOT(receiveCommand()));

	storeMessage(QString("Local %1: Running, hostName: %2, port: %3").arg(deviceName).arg(QHostInfo::localHostName()).arg(deviceServer->serverPort()), false);
}

LocalDevice::~LocalDevice()
{
	emit device_message(QString("Local %1: stopped").arg(device_name));
}

void LocalDevice::receiveCommand()
{
	QTcpSocket * socket = deviceServer->nextPendingConnection();
	connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

	QDataStream in(socket);

	if(!socket->waitForReadyRead(timeout)){
		emit device_message(QString("LOCAL %1 ERROR: receiveCommand: socket->waitForReadyRead: %2").arg(device_name).arg(socket->errorString()));
		emit device_fail();
		return;
	}

	QByteArray com = socket->readAll();
	QString command = QString::fromUtf8(com);

	emit device_message(QString("Local %1: received command: %2").arg(device_name).arg(command));
	
	//connect the devices to this signal to do somthing with the command
	emit newCommand(command, socket);
	/////////////////////////////////////////////////////////////////

	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->disconnectFromHost();

}
