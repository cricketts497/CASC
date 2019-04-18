#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include <QSerialPort>

#include "include/LocalDataDevice.h"

class SerialDevice : public LocalDataDevice
{
	Q_OBJECT
	
public:
	SerialDevice(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent = nullptr);
	
public slots:
    void queueSerialCommand(QString command);
	void stop_device();
	
signals:
	void newSerialResponse(QString response);
    void serialComFinished();
		
protected:
    bool openSerialPort();

    void setBaudRate(int rate);
    void setDataBits(int bits);
    void setParity(int parity);
    void setStopBits(int bits);
    void setFlowControl(int type);
    
    QQueue<QString> serialCommandQueue;
    bool writeCommand(QString command, bool response=false);

private slots:
	void readResponse();
	
	void serialTimeout();
	void serialError();
	
private:
    QSerialPort * serial_port = nullptr;
    
    QTimer * serial_timer = nullptr;
	const int serial_timeout;
    bool expectResponse;
};

#endif //SERIAL_DEVICE_H