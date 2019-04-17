#ifndef CASC_DEVICE_H
#define CASC_DEVICE_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QTimer>
#include <QHostAddress>

#include "include/CascConfig.h"
  
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
    const char * okMessage;
    const char * failMessage;

	bool device_failed;
	
private slots:
    void setFailed();
    
private:
    const int connection_timeout;

	QTextStream messages;
	QString messages_string;

};

#endif // CASC_DEVICE_H