#ifndef CASC_DEVICE_H
#define CASC_DEVICE_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QTextStream>

class CascConfig;

class CascDevice : public QObject
{
	Q_OBJECT
public:
	CascDevice(QString deviceName, CascConfig * config, QObject * parent = nullptr);
	void sendMessages();

signals:
	void device_message(QString message);
	void device_fail();

protected:
	void storeMessage(QString message, bool fail);

	const QString device_name;
	const int timeout;

	QString hostName;
	quint16 hostListenPort;
	quint16 hostDevicePort;

	const char * noDataMessage;

	bool device_failed;

private:
	QTextStream messages;
	QString messages_string;

};

#endif // CASC_DEVICE_H