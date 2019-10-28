#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include <QSerialPort>

#include <QMutex>
#include <QQueue>
#include <QThread>

// #include "include/LocalDataDevice.h"
#include "include/CascDevice.h"

class SerialDevice : public CascDevice
{
	Q_OBJECT
	
public:
	// SerialDevice(QStringList file_format, QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent = nullptr);
	SerialDevice(QString deviceName, CascConfig * config, QObject * parent = nullptr);
	
public slots:
    void queueSerialCommand(QString command);
	void stop_device();
	
signals:
	void newSerialResponse(QByteArray response);
    void serialComFinished();
		
protected:
    bool openSerialPort();

    void setBaudRate(int rate);
    void setDataBits(int bits);
    void setParity(int parity);
    void setStopBits(int bits);
    void setFlowControl(int type);
    
    void setSerialTimeout(int timeout);
    void setSerialResponseWait(int timeout);
    
    const int serial_timeout;
    
    
    QQueue<QString> serialCommandQueue;
    bool writeCommand(QByteArray command, bool response=false);
    
    const QByteArray noResponseMessage;
    
    void fullResponseReceived();

private slots:
    void urgentSerialCommand(QString command);
	void readResponse();
	
	void serialTimeout();
	void serialError();
	
private:
    QSerialPort * serial_port = nullptr;
    
    QTimer * serial_timer = nullptr;
    
    int serial_response_wait;
    
    bool expectResponse;
    bool commandInProgress;
    
    int missing_serial_response_count;
    const int missing_serial_response_limit;
    
    
};

#endif //SERIAL_DEVICE_H