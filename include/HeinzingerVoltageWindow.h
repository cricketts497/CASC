#ifndef HEINZINGER_VOLTAGE_WINDOW_H
#define HEINZINGER_VOLTAGE_WINDOW_H

#include "include/CascWidget.h"
#include "include/DeviceButton.h"

class HeinzingerVoltageWindow : public CascWidget
{
	Q_OBJECT
	
public:
	HeinzingerVoltageWindow(const QString voltage_file_path, QMutex * voltageFileMutex, uint maxVoltage=1, QWidget * parent=nullptr);
	
signals:
	void sendCommand(QString command);
	
private slots:
    void voltageChanged();
	void setVoltage(bool set);
    void setOutput(bool start);
    
    void readVoltage();
	
private:
	QSpinBox * voltageEdit;
    DeviceButton * voltageSetButton;
    DeviceButton * outputButton;
	QLabel * voltageReadback;
    
    QFile * voltage_file;
    QMutex * voltageFileMutex;
    
    quint64 current_applied_voltage;
	
};


#endif // HEINZINGER_VOLTAGE_WINDOW_H