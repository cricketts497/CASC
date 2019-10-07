#include "include/RemoteDevice.h"

RemoteDevice::RemoteDevice(QString deviceName, CascConfig * config, QObject * parent, bool askListener) :
CascDevice(deviceName, config, parent),
socket(new QTcpSocket(this)),
get_status_timer(new QTimer(this)),
get_status_timeout(1000),
askListener(askListener)
{
	if(device_failed)
		return;
	
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));

	connect(socket, SIGNAL(connected()), this, SLOT(writeCommand()));
	
    connect(socket, SIGNAL(readyRead()), this, SLOT(readResponse()));

	connect(connection_timer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));
	connect(socket, SIGNAL(disconnected()), connection_timer, SLOT(stop()));
    
    if(askListener){
        //send the remote command to start the device
        QString outString;
        QTextStream out(&outString);
        out << "start_" << deviceName;
        remoteDeviceCommand(out.readAll(), true);
    }
    
    connect(get_status_timer, SIGNAL(timeout()), this, SLOT(get_status()));
    connect(this, SIGNAL(device_fail()), get_status_timer, SLOT(stop()));
    
    get_status_timer->setInterval(get_status_timeout);
    get_status_timer->start();
}

void RemoteDevice::stop_device()
{
    get_status_timer->stop();
    if(socket->state() != QAbstractSocket::UnconnectedState){
        socket->disconnectFromHost();
    }
    
    //devices in sets may not need to ask the listener on the local machine to stop as linked to same device button
	if(device_failed || !askListener){
		emit stopped();
		return;
	}
    
    connect(socket, &QTcpSocket::disconnected, this, &CascDevice::stop_device);
    connect(this, &CascDevice::device_fail, this, &CascDevice::stop_device);
    
    //send the remote command to stop the device
    QString outString;
    QTextStream out(&outString);
    out << "stop_" << device_name;
    remoteDeviceCommand(out.readAll(), true);
}

void RemoteDevice::get_status()
{
    //don't ask for the status while sending a command
    if(!connection_timer->isActive())
        remoteDeviceCommand(QString(askStatusMessage));
}

//////////////////////////////////////////////////////////

//write device commands from the widgets
void RemoteDevice::remoteDeviceCommand(QString device_com, bool toListener)
{
    if(toListener){
        remoteDeviceCommandQueue.clear();
    }
    
    remoteDeviceCommandQueue.enqueue(device_com);
    
    if(toListener && socket->state() == QAbstractSocket::UnconnectedState){
        storeMessage(QString("Remote device: %1: Sending listener command %2").arg(device_name).arg(device_com), false);
        emit device_message(QString("Remote device: %1: Sending listener command %2").arg(device_name).arg(device_com));
        socket->connectToHost(hostAddress, hostListenPort);
    }else if(toListener){
        emit device_message(QString("REMOTE DEVICE ERROR: %1: Trying to send start/ stop command while still connected"));
        socket->abort();
    }else if(socket->state() == QAbstractSocket::UnconnectedState){
        socket->connectToHost(hostAddress, hostDevicePort);
    }
    //start or restart the timer
    connection_timer->start();
}

void RemoteDevice::writeCommand()
{    
    QString remoteCommand = remoteDeviceCommandQueue.dequeue();
    
    // emit device_message(QString("Remote device: %1: writing command %2").arg(device_name).arg(remoteCommand));
    
    socket->write(remoteCommand.toUtf8());
}


void RemoteDevice::readResponse()
{
    QByteArray resp = socket->readAll();
    
    if(resp.endsWith(failMessage)){
        emit device_message(QString("REMOTE %1 ERROR: fail message received from local").arg(device_name));
		emit device_fail();
        socket->disconnectFromHost();
        return;
    }
    
    if(resp.startsWith("Status")){
		setStatus(QString(resp));
    }else{
        emit newResponse(resp);
    }
    
    //next pending command
    if(remoteDeviceCommandQueue.isEmpty()){
        socket->disconnectFromHost();
    }else{
        writeCommand();
    }
}


//error handling
/////////////////////////////////////////////////////////////////
void RemoteDevice::connectionTimeout()
{
	storeMessage(QString("REMOTE ERROR: %1: Connection timeout").arg(device_name), true);
	emit device_message(QString("REMOTE ERROR: %1: Connection timeout").arg(device_name));

	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->abort();
    
    emit device_fail();
}

//including if remote host closes the connection
void RemoteDevice::socketError()
{	
    //listener closes the connection, ignore this
    if(socket->error() == QAbstractSocket::RemoteHostClosedError){ 
        return;
    }
    
	storeMessage(QString("REMOTE ERROR: %1: %2").arg(device_name).arg(socket->errorString()), true);
	emit device_message(QString("REMOTE ERROR: %1: %2").arg(device_name).arg(socket->errorString()));
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->abort();
    
    emit device_fail();
}
