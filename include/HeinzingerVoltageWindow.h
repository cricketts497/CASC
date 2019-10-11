#ifndef HEINZINGER_VOLTAGE_WINDOW_H
#define HEINZINGER_VOLTAGE_WINDOW_H

#include "include/CascWidget.h"
#include "QELineEdit.h"

class HeinzingerVoltageWindow : public CascWidget
{
	Q_OBJECT
	
public:
	HeinzingerVoltageWindow(QString deviceName, uint maxVoltage=1, qreal maxCurrent=0.01, QWidget * parent=nullptr);
	
    // void heinzingerDeviceOn(bool on);
    
// public slots:
    // void receiveHeinzingerStatus(QString status);
	
// private slots:
    // void voltageChanged();
    // void currentChanged();
	// void setVoltage();
    // void setCurrent();
    // void setOutput();
    
    // // void readVoltage();
	
// private:
	// QSpinBox * voltageEdit;
    // DeviceButton * voltageSetButton;
	// QLabel * voltageReadback;
    
    // QDoubleSpinBox * currentEdit;
    // DeviceButton * currentSetButton;
    
    // DeviceButton * outputButton;
    
    // QTimer * voltageReadTimer;
    // const int voltageReadTimeout;
    
    // QFile * voltage_file;
    // QMutex * voltageFileMutex;
    
    // quint64 applied_voltage_int;
    // quint64 applied_voltage_decimal;
    
    // bool output_on;
};


#endif // HEINZINGER_VOLTAGE_WINDOW_H