#ifndef REMOTE_DEVICE_H
#define REMOTE_DEVICE_H

#include "include/CascDevice.h"

#include <QtNetwork>

class RemoteDevice : public CascDevice
{
	Q_OBJECT
public:
	RemoteDevice(QString deviceName, CascConfig * config, QObject * parent=nullptr);
	~RemoteDevice();

protected slots:
	bool sendCommand(QString command);

protected:
	QTcpSocket * socket;
	
};


#endif // REMOTE_DEVICE_H