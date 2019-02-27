#include "include/LocalDevice.h"

LocalDevice::LocalDevice(QString deviceName, QString config_file_path, QObject * parent) :
CascDevice(deviceName, config_file_path, parent),
deviceServer(new QTcpServer(this))
{
	//start the device server
	if(!deviceServer->listen(QHostAddress::Any, hostDevicePort)){
		emit device_message(QString("LOCAL %1 ERROR: deviceServer->listen()").arg(deviceName));
		emit device_fail();
		return;
	}
	connect(deviceServer, SIGNAL(newConnection()), this, SLOT(receiveCommand()));
}

void LocalDevice::receiveCommand()
{
	QTcpSocket * socket = deviceServer->nextPendingConnection();
	connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

	QDataStream in(socket);

	if(!socket->waitForReadyRead(timeout)){
		emit device_message(QString("LOCAL %1 ERROR: receiveCommand: socket->waitForReadyRead, %1: %2").arg(device_name).arg(receiving_socket->peerName()).arg(receiving_socket->errorString()));
		emit device_fail();
		return;
	}

	QString command;
	in >> command;

	emit device_message(QString("Local %1: received command from %2: %3").arg(device_name).arg(socket->peerName()).arg(command));
	
	//connect the devices to this signal to do somthing with the command
	emit newCommand(command, socket);
	/////////////////////////////////////////////////////////////////

	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->disconnectFromHost();
}
