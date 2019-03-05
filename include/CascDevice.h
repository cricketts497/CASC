#ifndef CASC_DEVICE_H
#define CASC_DEVICE_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QTimer>
#include <QHostAddress>

class CascConfig;

class CascDevice : public QObject
{
	Q_OBJECT
public:
	CascDevice(QString deviceName, CascConfig * config, QObject * parent = nullptr);
	~CascDevice();
	void sendMessages();

public slots:
	void stop_device();

signals:
	void device_message(QString message);
	void device_fail();
	
	void stopped();

protected:
	void storeMessage(QString message, bool fail);

	const QString device_name;
	
	QTimer * connection_timer = nullptr;

	QString hostAddress;
	quint16 hostListenPort;
	quint16 hostDevicePort;

	const char * noDataMessage;

	bool device_failed;
	
private:
	QTextStream messages;
	QString messages_string;
	
	const int timeout;
};

#endif // CASC_DEVICE_H