
#include "include/LocalDevice.h"

/*!
    \class LocalDevice
    \brief Physical or virtual backend devices in CASC on this PC.
    \inherits CascDevice
    
    Initialisation: Creates a QTcpServer deviceServer for receiving commands from remote PCs connected to hostDevicePort.
    
*/
  
LocalDevice::LocalDevice(QString deviceName, CascConfig * config, QObject * parent) :
CascDevice(deviceName, config, parent),
deviceServer(new QTcpServer(this)),
deviceBusy(false)
{
	if(device_failed){
		return;
    }

	if(!deviceServer->listen(QHostAddress::Any, hostDevicePort)){
		storeMessage(QString("LOCAL DEVICE ERROR: %1: deviceServer->listen()").arg(deviceName), true);
		return;
	}
	connect(deviceServer, SIGNAL(newConnection()), this, SLOT(newCon()));
	
	connect(connection_timer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));

	storeMessage(QString("Local %1: Running, port: %2").arg(deviceName).arg(deviceServer->serverPort()), false);
}

/*!
Run when deviceServer emits \l QTcpServer::newConnection() upon receiving a connection from the device on a remote PC. The \l {QTcpSocket::}readyRead() signal is connected to receiveCommand(). The server pauses accepting new connections until the messageReceived signal is emitted.
*/
void LocalDevice::newCon()
{
    if(deviceBusy){
        return;
    }
    
	socket = deviceServer->nextPendingConnection();
    deviceBusy = true;
    
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
	
	connect(socket, SIGNAL(readyRead()), this, SLOT(receiveCommand()));
	
    connect(socket, SIGNAL(disconnected()), this, SLOT(messageReceived()));
    connect(socket, SIGNAL(disconnected()), connection_timer, SLOT(stop()));
	connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
	
	connection_timer->start();
}

// /*!
    // Slot connected to local \l CascWidget via intermediate \l MainWindow member function. Emits signal newCommand() connected to slots in child classes \sa RemoteDevice::deviceCommand().
// */
// void LocalDevice::localDeviceCommand(QString command)
// {
    // emit newLocalCommand(command);
// }

/*!
    Run when the \l {QTcpSocket::}readyRead() signal is emitted by socket. Reads the command and emits it through newCommand()
*/
void LocalDevice::receiveCommand()
{	
    if(device_failed){
        socket->write(failMessage);
        return;
    }

	QByteArray com = socket->readAll();
	QString command = QString::fromUtf8(com);
    
    //listener commands
    if(command.split("_").first() == QString("start") || command.split("_").first() == QString("stop")){
        socket->write(okMessage);
        return;
    }
    
    //status command
    if(command == QString(askStatusMessage)){
        socket->write(getStatus().toUtf8());
        return;
    }
    
    // emit device_message(QString("Local device: %1: received device command %2").arg(device_name).arg(command));
    
	//connect the devices to this signal to do something with the command
	// emit newRemoteCommand(command);
}

void LocalDevice::messageReceived()
{
    deviceBusy = false;
    if(deviceServer->hasPendingConnections())
        newCon();
}

//error handling
///////////////////////////////////////////////////////////////
void LocalDevice::connectionTimeout()
{
	storeMessage(QString("LOCAL %1 ERROR: Connection timeout").arg(device_name), true);
	emit device_message(QString("LOCAL %1 ERROR: Connection timeout").arg(device_name));
	emit device_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->write(failMessage);
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
		socket->abort();
}