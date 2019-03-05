#ifndef LISTENER_H
#define LISTENER_H

#include <QObject>
#include <QHostAddress>

class QNetworkSession;
class QTcpServer;
class QTcpSocket;
class QTimer;

class CascConfig;

class Listener : public QObject
{
	Q_OBJECT

public:
	Listener(CascConfig * config, QObject * parent = nullptr);
	~Listener();

	void start();

signals:
	void listener_message(QString message);
	void listener_fail();

	void toggle_device_command(QString deviceName, bool start);

private slots:
	void sessionOpened();
	void newCom();
	void receiveCommand();
	
	void connectionTimeout();
	void socketError();

private:
	CascConfig * config = nullptr;

	QNetworkSession * networkSession = nullptr;
	
	const int timeout;
	QTimer * connection_timer;

	//server to receive commands
	QTcpServer * tcpServer = nullptr;
	quint16 listenPort;
	QTcpSocket * socket = nullptr;

};

#endif // LISTENER_H