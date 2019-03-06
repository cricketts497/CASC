#ifndef REMOTE_HEINZINGER_PS_H
#define REMOTE_HEINZINGER_PS_H

#include "include/RemoteDevice.h"

class RemoteHeinzingerPS : public RemoteDevice
{
	Q_OBJECT

public:
	RemoteHeinzingerPS(QString deviceName, CascConfig * config, QObject * parent=nullptr);
	
signals:
	void newTrueVoltage(int voltage);
	
public slots:
	void set_voltage(uint voltage);
	
private slots:
	void askVoltage();
	void receiveVoltage();	
	
	void heinzingerError();
	
private:
	QTimer * voltage_query_timer = nullptr;
	const int voltage_query_timeout;	
};


#endif // REMOTE_HEINZINGER_PS_H