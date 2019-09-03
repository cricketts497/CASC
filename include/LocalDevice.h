#ifndef LOCAL_DEVICE_H
#define LOCAL_DEVICE_H

#include "include/CascDevice.h"

#include <QtNetwork>

class LocalDevice : public CascDevice
{
	Q_OBJECT
	
public:
	LocalDevice(QString deviceName, CascConfig * config, QObject * parent=nullptr);
	
// public slots:
    // void localDeviceCommand(QString command);
    
signals:
    void newLocalCommand(QString command);
    void newRemoteCommand(QString command);
    
protected:
	QTcpSocket * socket = nullptr;

private slots:
	void newCon();
	void receiveCommand();
	void messageReceived();
    	
	void connectionTimeout();
	void socketError();

private:
	QTcpServer * deviceServer = nullptr;
    bool deviceBusy;

};

#endif // LOCAL_DEVICE_H