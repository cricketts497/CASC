#ifndef REMOTE_DEVICE_H
#define REMOTE_DEVICE_H

#include "include/CascDevice.h"

#include <QtNetwork>

class RemoteDevice : public CascDevice
{
	Q_OBJECT
public:
	RemoteDevice(QString deviceName, CascConfig * config, QObject * parent=nullptr);
	
public slots:
    void deviceCommand(QString command);
	void stop_device();

protected:
	QTcpSocket * socket;
    
    QString remoteCommand;
	
private slots:
	void writeCommand();
    void readResponse();
	
	void connectionTimeout();
	void socketError();
};


#endif // REMOTE_DEVICE_H