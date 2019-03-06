#include <include/RemoteDevice.h>

RemoteDevice::RemoteDevice(QString deviceName, CascConfig * config, QObject * parent) :
CascDevice(deviceName, config, parent),
socket(new QTcpSocket(this))
{
	if(device_failed)
		return;
	
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
	connect(socket, SIGNAL(connected()), this, SLOT(writeCommand()));
	
	connect(connection_timer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));
	connect(socket, SIGNAL(connected()), connection_timer, SLOT(stop()));
	connect(socket, SIGNAL(disconnected()), connection_timer, SLOT(stop()));
	
	//send the command to start the device
	QString outString;
	QTextStream out(&outString);
	out << "start_" << deviceName;
	command = out.readAll();
	
	socket->connectToHost(hostAddress, hostListenPort);
	connection_timer->start();
}

void RemoteDevice::stop_device()
{
	if(device_failed){
		emit stopped();
		return;
	}

	//send the command to stop the device
	QString outString;
	QTextStream out(&outString);
	out << "stop_" << device_name;
	command = out.readAll();

	socket->connectToHost(hostAddress, hostListenPort);
	connection_timer->start();
	
	connect(socket, SIGNAL(disconnected()), this, SLOT(emitStopped()));
}

void RemoteDevice::emitStopped()
{
	emit stopped();
}

void RemoteDevice::writeCommand()
{
	socket->write(command.toUtf8());
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

void RemoteDevice::socketError()
{
	//ignore if the remote host closes the connection
	if(socket->error() == QAbstractSocket::RemoteHostClosedError)
		return;
	
	storeMessage(QString("REMOTE %1 ERROR: %2").arg(device_name).arg(socket->errorString()), true);
	emit device_message(QString("REMOTE %1 ERROR: %2").arg(device_name).arg(socket->errorString()));
	emit device_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->disconnectFromHost();
}




