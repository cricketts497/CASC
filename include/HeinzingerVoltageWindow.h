#ifndef HEINZINGER_VOLTAGE_WINDOW_H
#define HEINZINGER_VOLTAGE_WINDOW_H

#include "include/CascWidget.h"
#include "QESpinBox.h"
#include "QELabel.h"
#include "QELineEdit.h"

class HeinzingerVoltageWindow : public CascWidget
{
	Q_OBJECT
	
public:
	HeinzingerVoltageWindow(QString deviceName, uint maxVoltage=1, qreal maxCurrent=0.01, QWidget * parent=nullptr);

private slots:
    void setOutputButtonState();
    void toggleOutputCommanded();
    void outputSetpointTimeoutMessage();

private:    
    QTimer * outputSetpointTimer;
    const int outputSetpointTimeout;
    
    QPushButton * outputButton;
    QELineEdit * outputCommanded;
    QELineEdit * outputSetpoint;
};


#endif // HEINZINGER_VOLTAGE_WINDOW_H