#ifndef LISTENER_H
#define LISTENER_H

#include <QObject>
#include <QHostAddress>

class QNetworkSession;
class QTcpServer;
class QTcpSocket;
class QTimer;

class Listener : public QObject
{
	Q_OBJECT

public:
	Listener(quint16 server_port=11111, QObject * parent = nullptr);
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
	QNetworkSession * networkSession = nullptr;
	
	const int timeout;
	QTimer * connection_timer;

	//server to receive commands
	QTcpServer * tcpServer = nullptr;
	const quint16 server_port;
	QTcpSocket * socket = nullptr;

};

#endif // LISTENER_H