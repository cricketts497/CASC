#ifndef HEINZINGER_VOLTAGE_WINDOW_H
#define HEINZINGER_VOLTAGE_WINDOW_H

#include "include/CascWidget.h"

class HeinzingerVoltageWindow : public CascWidget
{
	Q_OBJECT
	
public:
	HeinzingerVoltageWindow(uint maxVoltage=1, QWidget * parent=nullptr);
	
signals:
	void set_voltage(uint voltage);
	
public slots:
	void readbackVoltage(qreal voltage);
	
private slots:
	void changeVoltage();
	
private:
	QSpinBox * voltageEdit;
	
	QLabel * voltageReadback;
	
};


#endif // HEINZINGER_VOLTAGE_WINDOW_H