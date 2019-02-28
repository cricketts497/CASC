#include <include/RemoteDevice.h>

RemoteDevice::RemoteDevice(QString deviceName, CascConfig * config, QObject * parent) :
CascDevice(deviceName, config, parent),
socket(new QTcpSocket(this))
{
	if(device_failed)
		return;

	//send the command to start the device
	// QByteArray block;
	// QDataStream out(&block, QIODevice::WriteOnly);
	QString outString;
	QTextStream out(&outString);

	out << "start_" << deviceName;

	socket->connectToHost(hostName, hostListenPort);
	if(!socket->waitForConnected(timeout)){
		storeMessage(QString("REMOTE %1 ERROR: init: connect to listener, %2, port %3: %4").arg(deviceName).arg(hostName).arg(hostListenPort).arg(socket->errorString()), true);
		return;
	}

	socket->write(out.readAll().toUtf8());

	if(!socket->waitForDisconnected(timeout)){
		storeMessage(QString("REMOTE %1 ERROR: init: disconnect from listener, %2, port %3: %4").arg(deviceName).arg(hostName).arg(hostListenPort).arg(socket->errorString()), true);
	}
}

RemoteDevice::~RemoteDevice()
{
	if(device_failed)
		return;

	//send the command to stop the device
	// QByteArray block;
	// QDataStream out(&block, QIODevice::WriteOnly);
	QString outString;
	QTextStream out(&outString);

	out << "stop_" << device_name;

	socket->connectToHost(hostName, hostListenPort);
	if(!socket->waitForConnected(timeout)){
		emit device_message(QString("REMOTE %1 ERROR: destroy: connect to listener, %2, port %3: %4").arg(device_name).arg(hostName).arg(hostListenPort).arg(socket->errorString()));
		emit device_fail();
		return;
	}

	socket->write(out.readAll().toUtf8());

	if(!socket->waitForDisconnected(timeout)){
		emit device_message(QString("REMOTE %1 ERROR: destroy: disconnect from listener, %2, port %3: %4").arg(device_name).arg(hostName).arg(hostListenPort).arg(socket->errorString()));
		emit device_fail();
	}
}

bool RemoteDevice::sendCommand(QString command)
{
	// QByteArray block;
	// QDataStream out(&block, QIODevice::WriteOnly);

	// out << command.toLatin1();

	socket->connectToHost(hostName, hostDevicePort);
	if(!socket->waitForConnected(timeout)){
		emit device_message(QString("REMOTE %1 ERROR: sendCommand: waitForConnection, %2: %3").arg(device_name).arg(hostName).arg(socket->errorString()));
		emit device_fail();
		return false;
	}
	
	emit device_message(QString("Remote %1: sending %2 command").arg(device_name).arg(command));
	socket->write(command.toUtf8());

	return true;
}