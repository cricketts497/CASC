#ifndef REMOTE_DEVICE_H
#define REMOTE_DEVICE_H

#include "include/CascDevice.h"

class QTcpSocket;

class RemoteDevice : public CascDevice
{
	Q_OBJECT
public:
	RemoteDevice(QString deviceName, QString config_file_path, QObject * parent=nullptr);

protected slots:
	bool sendCommand(QString command);

protected:
	QTcpSocket * socket;
	
};


#endif // REMOTE_DEVICE_H