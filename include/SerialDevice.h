#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include <QSerialPort>

#include "include/LocalDataDevice.h"

class SerialDevice : public LocalDataDevice
{
	Q_OBJECT
	
public:
	SerialDevice(QStringList file_format, QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent = nullptr);
	
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
    
    const int serial_timeout;
    const int serial_response_wait;
    
    QQueue<QString> serialCommandQueue;
    bool writeCommand(QByteArray command, bool response=false);
    
    const QByteArray noResponseMessage;
    
    void fullResponseReceived();

private slots:
	void readResponse();
	
	void serialTimeout();
	void serialError();
	
private:
    QSerialPort * serial_port = nullptr;
    
    QTimer * serial_timer = nullptr;
    
    bool expectResponse;
    bool commandInProgress;
    
    int missing_serial_response_count;
    const int missing_serial_response_limit;
};

#endif //SERIAL_DEVICE_H