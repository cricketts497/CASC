#ifndef LOCAL_DEVICE_H
#define LOCAL_DEVICE_H

#include "include/CascDevice.h"

class QTcpServer;
class QTcpSocket;

class LocalDevice : public CascDevice
{
	Q_OBJECT
	
public:
	LocalDevice(QString deviceName, QObject * parent=nullptr);

signals:
	void newCommand(QString command, QTcpSocket * socket);

private:
	QTcpServer * deviceServer = nullptr;

};

#endif // LOCAL_DEVICE_H