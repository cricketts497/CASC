#ifndef LISTENER_H
#define LISTENER_H

#include <QObject>
#include <QHostAddress>

class QNetworkSession;
class QTcpServer;
class QTcpSocket;

class Listener : public QObject
{
	Q_OBJECT

public:
	Listener(quint16 port=11111, QObject * parent = nullptr);
	~Listener();

	void start();
	void sendCommand(QString command, QString host, quint16 port);

signals:
	void listener_message(QString message);
	void listener_fail();

private slots:
	void sessionOpened();
	void receiveCommand();

private:
	QNetworkSession * networkSession = nullptr;
	const int timeout;

	//server to receive commands
	QTcpServer * tcpServer = nullptr;
	const quint16 server_port;

	//socket to send commands
	QTcpSocket * sending_socket = nullptr;

};

#endif // LISTENER_H