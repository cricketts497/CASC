#include <QtNetwork>
#include <QTimer>

#include "include/Listener.h"

Listener::Listener(quint16 server_port, QObject * parent) : 
QObject(parent),
timeout(3000),
server_port(server_port)
{
	connection_timer = new QTimer(this);
	connection_timer->setSingleShot(true);
	connection_timer->setInterval(timeout);
}

Listener::~Listener()
{
	emit listener_message(QString("Listener: Stopped"));
}

void Listener::start()
{
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
	if(!tcpServer->listen(QHostAddress::Any, server_port)){
		emit listener_message(QString("LISTENER ERROR: tcpServer->listen(): unable to start server"));
		emit listener_fail();
		return;
	}
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newCom()));

	connect(connection_timer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));

	// QString ipAddress;
	// QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
	// //use the first non localhost IPv4 address
	// for(int i=0; i<ipAddressesList.size(); i++){
	// 	if(ipAddressesList.at(i) != QHostAddress::LocalHost && ipAddressesList.at(i).toIPv4Address()){
	// 		ipAddress = ipAddressesList.at(i).toString();
	// 		break;
	// 	}
	// }
	// //if we don't find one, use the localhost
	// if(ipAddress.isEmpty())
	// 	ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

	emit listener_message(QString("Listener: Running, port: %1").arg(tcpServer->serverPort()));

}

void Listener::newCom()
{
	socket = tcpServer->nextPendingConnection();
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

	if(command_list.first() == QString("start"))
		emit toggle_device_command(command_list.at(1), true);
	else if(command_list.first() == QString("stop"))
		emit toggle_device_command(command_list.at(1), false);

}

//Error handling
///////////////////////////////////////////////////////////////////
void Listener::connectionTimeout()
{
	emit listener_message(QString("LISTENER ERROR: connection timeout"));
	emit listener_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->disconnectFromHost();
}

void Listener::socketError()
{
	//ignore if the remote host closes the connection
	if(socket->error() == QAbstractSocket::RemoteHostClosedError)
		return;
	
	emit listener_message(QString("LISTENER ERROR: %1").arg(socket->errorString()));
	emit listener_fail();
	
	if(socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState)
		socket->disconnectFromHost();
}
