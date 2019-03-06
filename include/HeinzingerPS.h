#ifndef HEINZINGER_PS_H
#define HEINZINGER_PS_H

#include "include/SerialDevice.h"

class HeinzingerPS : public SerialDevice
{
	Q_OBJECT
	
public:
	HeinzingerPS(QString deviceName, QString file_path, QMutex * file_mutex, CascConfig * config, QObject * parent=nullptr);
	
signals:
	void newTrueVoltage(qreal voltage);
	
public slots:
	void setVoltage(uint voltage);
	
private slots:
	void sendSetVolt();
	void sendQueryAppliedVolt();
	void applyVoltage(QString response);
	void checkAverages(QString response);
	
	void queryVoltage();
	void readbackVoltage(QString response);
	
	void remoteVoltageCommand(QString command);
	
	void heinzingerError();
	
	void bytesWrite();
	
private:
	QFile * voltage_file;
	QMutex * file_mutex;
	
	QDateTime * time;
	QTimer * voltage_query_timer;
	const int voltage_query_timeout;
	
	uint voltage_setpoint;
	qreal true_voltage;
	const uint nAverages;
	
	QMetaObject::Connection connection;

};


#endif // HEINZINGER_PS_H