#include <include/RemoteDevice.h>

#include <QtNetwork>

RemoteDevice::RemoteDevice(QString deviceName, QString config_file_path, QObject * parent) :
CascDevice(deviceName, config_file_path, parent),
socket(new QTcpSocket(this))
{
	//send the command to start the device
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);

	out << "start_" << deviceName;

	socket->connectToHost(hostName, hostListenPort);
	if(!waitForConnected(timeout)){
		emit device_message(QString("REMOTE %1 ERROR: connect to listener, %2, port %3: %4").arg(deviceName).arg(hostName).arg(hostListenPort).arg(errorString()));
		emit device_fail();
		return;
	}

	socket->write(block);

	if(!waitForDisconnected(timeout)){
		emit device_message(QString("REMOTE %1 ERROR: disconnect from listener, %2, port %3: %4").arg(deviceName).arg(hostName).arg(hostListenPort).arg(errorString()));
		emit device_fail();
	}
}

void RemoteDevice::sendCommand(QString command)
{
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);

	out << command;

	socket->connectToHost(hostName, hostDevicePort);
	if(!socket->waitForConnected(timeout)){
		emit device_message(QString("REMOTE %1 ERROR: sendCommand: waitForConnection, %2: %3").arg(device_name).arg(hostName).arg(socket->errorString()));
		emit device_fail();
		return;
	}

	socket->write(block);

}