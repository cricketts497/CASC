#ifndef CASC_DEVICE_H
#define CASC_DEVICE_H

// #include <QObject>
// #include <QFile>
// #include <QDataStream>
// #include <QTextStream>
#include <QTimer>
// #include <QHostAddress>

#include "include/CascConfig.h"
  
class CascDevice : public QObject
{
	Q_OBJECT
public:
	CascDevice(QString deviceName, QObject * parent = nullptr);
	~CascDevice();
    
	void sendMessages();

public slots:
	void stop_device();
    bool getDeviceFailed();
    
    void receiveWidgetCommand(QString);
    void receiveUrgentWidgetCommand(QString);

signals:
    void device_status(QString status);
    
    void newWidgetCommand(QString command);
    void newUrgentWidgetCommand(QString command);
    
	void device_message(QString message);
	void device_fail();
	
	void stopped();

protected:
    QString getStatus();
    void setStatus(QString status);
    
	void storeMessage(QString message, bool fail);

	const QString device_name;
	
	// QTimer * connection_timer = nullptr;

	// QString hostAddress;
	// quint16 hostListenPort;
	// quint16 hostDevicePort;

	// const char * noDataMessage;
    // const char * okMessage;
    // const char * failMessage;
    
    // const char * askStatusMessage;

	bool device_failed;
    	
private slots:
    void setFailed();
    void broadcastStatus();
    
private:
    QString deviceStatus;
    
    // const int connection_timeout;
    
    QTimer * broadcast_status_timer = nullptr;
    const int broadcast_status_timeout;

	QTextStream messages;
	QString messages_string;

};

#endif // CASC_DEVICE_H