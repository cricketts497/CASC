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
	CascConfig * casc_config = nullptr;

	QNetworkSession * networkSession = nullptr;
	
	const int timeout;
	QTimer * connection_timer = nullptr;

	//server to receive commands
	QTcpServer * tcpServer = nullptr;
	quint16 listenPort;
	QTcpSocket * socket = nullptr;
    
    bool listenerBusy;
    void freeListener();

};

#endif // LISTENER_H