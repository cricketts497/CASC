#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include <QSerialPort>

#include "include/LocalDevice.h"

class SerialDevice : public LocalDevice
{
	Q_OBJECT
	
public:
	SerialDevice(QString deviceName, CascConfig * config, QObject * parent = nullptr);
	
public slots:
	void stop_device();
	
signals:
	void newResponse(QString response);

protected:
	// void writeCommand(QString command);
	
	QSerialPort * serial_port = nullptr;
	
	QTimer * serial_timer = nullptr;
		
protected slots:
	void waitForResponse();

private slots:
	
	void readResponse();
	
	void serialTimeout();
	void serialError();
	
private:
	const int serial_timeout;
	
};

#endif //SERIAL_DEVICE_H