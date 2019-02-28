#include <QtNetwork>
// #include <QtCore>

#include "include/Listener.h"

Listener::Listener(quint16 server_port, QObject * parent) : 
QObject(parent),
timeout(3000),
server_port(server_port),
sending_socket(new QTcpSocket(this))
{

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
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(receiveCommand()));


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

	emit listener_message(QString("Listener: Running, hostName: %1, port: %2").arg(QHostInfo::localHostName()).arg(tcpServer->serverPort()));

}

void Listener::receiveCommand()
{
	QTcpSocket * receiving_socket = tcpServer->nextPendingConnection();
	connect(receiving_socket, SIGNAL(disconnected()), receiving_socket, SLOT(deleteLater()));

	QDataStream in(receiving_socket);
	
	if(!receiving_socket->waitForReadyRead(timeout)){
		emit listener_message(QString("LISTENER ERROR: receiveCommand: waitForReadyRead: %1").arg(receiving_socket->errorString()));
		emit listener_fail();
		return;
	}

	QByteArray com = receiving_socket->readAll();
	QString command = QString::fromUtf8(com);

	receiving_socket->disconnectFromHost();

	emit listener_message(QString("Listener: received command: %1").arg(command));

	//deal with start/ stop device commands
	QStringList command_list = command.split("_");

	if(command_list.first() == QString("start"))
		emit toggle_device_command(command_list.at(1), true);
	else if(command_list.first() == QString("stop"))
		emit toggle_device_command(command_list.at(1), false);

}
