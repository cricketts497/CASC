#ifndef REMOTE_DEVICE_H
#define REMOTE_DEVICE_H

#include "include/CascDevice.h"

#include <QtNetwork>

class RemoteDevice : public CascDevice
{
	Q_OBJECT
public:
	RemoteDevice(QString deviceName, CascConfig * config, QObject * parent=nullptr);
	
signals:
    void newResponse(QByteArray resp);
    
public slots:
    void remoteDeviceCommand(QString command, bool toListener=false);
	void stop_device();

protected:
    QTcpSocket * socket;
    QQueue<QString> remoteDeviceCommandQueue;
    
private slots:
    void get_status();

	void writeCommand();
    
    void readResponse();
    	
	void connectionTimeout();
	void socketError();
    
private:
    QTimer * get_status_timer = nullptr;
    const int get_status_timeout;
};

#endif // REMOTE_DEVICE_H