#ifndef AGILENT_TV301_STATUS_WINDOW
#define AGILENT_TV301_STATUS_WINDOW

#include "include/CascWidget.h"
#include "include/ParamReadout.h"

const int nAgilentTV301Pumps = 3;

class AgilentTV301StatusWindow : public CascWidget
{
    Q_OBJECT
public:
    AgilentTV301StatusWindow(QStringList pump_names, QWidget * parent=nullptr);

public slots:
    void receiveAgilentTV301Status(QString status);

private:
    const QStringList pump_names;
    const int nominal_speed;
    
    ParamReadout * drives[nAgilentTV301Pumps];
    ParamReadout * temperatures[nAgilentTV301Pumps];
    ParamReadout * statuses[nAgilentTV301Pumps];
};

#endif // AGILENT_TV301_STATUS_WINDOW