#ifndef LOCAL_DEVICE_H
#define LOCAL_DEVICE_H

#include "include/CascDevice.h"

#include <QtNetwork>

class LocalDevice : public CascDevice
{
	Q_OBJECT
	
public:
	LocalDevice(QString deviceName, CascConfig * config, QObject * parent=nullptr);
	~LocalDevice();

signals:
	void newCommand(QString command, QTcpSocket * socket);

private slots:
	void receiveCommand();

private:
	QTcpServer * deviceServer = nullptr;

};

#endif // LOCAL_DEVICE_H