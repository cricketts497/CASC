#include <QtNetwork>
#include <QTimer>

#include "include/Listener.h"
#include "include/CascConfig.h"

Listener::Listener(CascConfig * config, QObject * parent) : 
QObject(parent),
casc_config(config),
timeout(3000),
connection_timer(new QTimer(this)),
listenerBusy(false)
{
	connection_timer->setSingleShot(true);
	connection_timer->setInterval(timeout);
}

Listener::~Listener()
{
	emit listener_message(QString("Listener: Stopped"));
}

void Listener::start()
{
	QStringList listen_config = casc_config->getDevice(QString("listener"));
	if(listen_config.isEmpty() || listen_config.size() != 2){
		emit listener_message(QString("LISTENER ERROR: listener not found in config"));
		emit listener_fail();
		return;
	}
    
    bool conv_ok;
	listenPort = listen_config.at(1).toUShort(&conv_ok);
	if(listenPort == 0 || !conv_ok){
        emit listener_message(QString("LISTENER ERROR: TCP listener port is invalid in config"));
		emit listener_fail();
		return;
	}    
    
	//check if the system requires a network session before network operations can be performed
	QNetworkConfigurationManager manager;
	if(manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired){
		// Get saved network configuration
		QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
		settings.beginGroup(QLatin1String("QtNetwork"));
		const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
		settings.endGroup();

		// If the saved network configuration is not currently discovered use the system default
		QNetworkConfiguration config = manager.configurationFromIdentifier(id);
		if ((config.state() & QNetworkConfiguration::Discovered) !=
				QNetworkConfiguration::Discovered) {
				config = manager.defaultConfiguration();
		}

		networkSession = new QNetworkSession(config, this);
		connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

		emit listener_message(QString("Listener: Opening network session"));
		networkSession->open();
	}else{
		sessionOpened();
	}
}

void Listener::sessionOpened()
{
	//saved the used network config
	if(networkSession){
		QNetworkConfiguration config = networkSession->configuration();
		QString id;
		if (config.type() == QNetworkConfiguration::UserChoice)
				id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
		else
				id = config.identifier();

		QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
		settings.beginGroup(QLatin1String("QtNetwork"));
		settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
		settings.endGroup();
	}

	tcpServer = new QTcpServer(this);
	if(!tcpServer->listen(QHostAddress::Any, listenPort)){
		emit listener_message(QString("LISTENER ERROR: tcpServer->listen(%1): unable to start server").arg(listenPort));
		emit listener_fail();
		return;
	}
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newCom()));

	connect(connection_timer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));

	emit listener_message(QString("Listener: Running, port: %1").arg(tcpServer->serverPort()));
}

void Listener::newCom()
{
    if(listenerBusy){
        emit listener_message(QString("Listener: New connection attempt received while busy"));
        return;
    }
    
	socket = tcpServer->nextPendingConnection();
    listenerBusy = true;
    
	connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));    
    
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
    
	connect(socket, SIGNAL(readyRead()), this, SLOT(receiveCommand()));
	connect(socket, SIGNAL(readyRead()), connection_timer, SLOT(stop()));
    
	connection_timer->start();
}

void Listener::receiveCommand()
{
	QDataStream in(socket);

	QByteArray com = socket->readAll();
	QString command = QString::fromUtf8(com);

	socket->disconnectFromHost();

	emit listener_message(QString("Listener: received command: %1").arg(command));

	//deal with start/ stop device commands
	QStringList command_list = command.split("_");

	if(command_list.first() == QString("start")){
        for(int i=1; i<command_list.length(); i++){
            emit toggle_device_command(command_list.at(i), true);
        }
	}else if(command_list.first() == QString("stop")){
		for(int i=1; i<command_list.length(); i++){
            emit toggle_device_command(command_list.at(i), false);
        }
    }
    freeListener();
}

void Listener::freeListener()
{
    listenerBusy = false;
    if(tcpServer->hasPendingConnections()){
        newCom();
    }
}

//Error handling
///////////////////////////////////////////////////////////////////
void Listener::connectionTimeout()
{
	emit listener_message(QString("LISTENER ERROR: connection timeout"));
	emit listener_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->abort();
}

void Listener::socketError()
{
	//ignore if the remote host closes the connection
	if(socket->error() == QAbstractSocket::RemoteHostClosedError)
		return;
	
	emit listener_message(QString("LISTENER ERROR: %1").arg(socket->errorString()));
	emit listener_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->abort();
}
