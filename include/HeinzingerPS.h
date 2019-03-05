#ifndef HEINZINGER_PS_H
#define HEINZINGER_PS_H

#include "include/SerialDevice.h"

class HeinzingerPS : public SerialDevice
{
	Q_OBJECT
	
public:
	HeinzingerPS(CascConfig * config, QObject * parent=nullptr);
	
public slots:
	void setVoltage(uint voltage);
	void queryVoltage();
	
private slots:
	void applyVoltage(QString response);
	void readbackVoltage();
	
private:
	uint voltage_setpoint;
	uint true_voltage;
	uint nAverages;
	
	QMetaObject::Connection connection;
};


#endif // HEINZINGER_PS_H