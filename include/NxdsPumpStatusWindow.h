#ifndef NXDS_PUMP_STATUS_WINDOW_H
#define NXDS_PUMP_STATUS_WINDOW_H

#include "include/CascWidget.h"
#include "include/ParamReadout.h"

const int nPumps = 1;

class NxdsPumpStatusWindow : public CascWidget
{
    Q_OBJECT
public:
    NxdsPumpStatusWindow(QStringList pump_names, QWidget * parent=nullptr);
    
public slots:
    void receiveNxdsStatus(QString status);
    
private:
    const QStringList pump_names;
    const int nominal_speed;

    ParamReadout * speeds[nPumps];
    ParamReadout * temperatures[nPumps];
    ParamReadout * statuses[nPumps];
};



#endif // NXDS_PUMP_STATUS_WINDOW_H