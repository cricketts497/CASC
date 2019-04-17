#ifndef HEINZINGER_VOLTAGE_WINDOW_H
#define HEINZINGER_VOLTAGE_WINDOW_H

#include "include/CascWidget.h"
#include "include/DeviceButton.h"

class HeinzingerVoltageWindow : public CascWidget
{
	Q_OBJECT
	
public:
	HeinzingerVoltageWindow(const QString voltage_file_path, QMutex * voltageFileMutex, uint maxVoltage=1, qreal maxCurrent=1.0, QWidget * parent=nullptr);
	
    void heinzingerDeviceOn(bool on);
	
private slots:
    void voltageChanged();
    void currentChanged();
	void setVoltage(bool set);
    void setCurrent(bool set);
    void setOutput(bool start);
    
    void readVoltage();
	
private:
	QSpinBox * voltageEdit;
    DeviceButton * voltageSetButton;
	QLabel * voltageReadback;
    
    QDoubleSpinBox * currentEdit;
    DeviceButton * currentSetButton;
    
    DeviceButton * outputButton;
    
    QTimer * voltageReadTimer;
    const int voltageReadTimeout;
    
    QFile * voltage_file;
    QMutex * voltageFileMutex;
    
    quint64 applied_voltage_int;
    quint64 applied_voltage_decimal;
};


#endif // HEINZINGER_VOLTAGE_WINDOW_H