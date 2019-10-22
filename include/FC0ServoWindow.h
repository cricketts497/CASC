#ifndef FC0_SERVO_WINDOW
#define FC0_SERVO_WINDOW

#include "include/CascWidget.h"
#include "QELineEdit.h"

class FC0ServoWindow : public CascWidget
{
    Q_OBJECT
public:
    FC0ServoWindow(QString deviceName, QWidget * parent = nullptr);
    
private slots:
    void setButtonState();
    void toggleStateCommanded();
    void stateSetpointTimeoutMessage();

private:    
    QPushButton * button;
    
    QTimer * stateSetpointTimer;
    const int stateSetpointTimeout;
    
    QELineEdit * stateCommanded;
    QELineEdit * stateSetpoint;

};


#endif // FC0_SERVO_WINDOW