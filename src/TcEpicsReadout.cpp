#include "include/TcEpicsReadout.h"

TcEpicsReadout::TcEpicsReadout(QWidget * parent) : 
CascWidget("Thermocouples", parent)
{
    QWidget * widget = new QWidget(this);
	setWidget(widget);
    
    QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);
    
    for(int i=1; i<8; i++){
        const QString varName = QString("CRISTCs:TC%1").arg(i);
        
        QLabel * varLabel = new QLabel(varName, this);
        layout->addWidget(varLabel, i-1,0);
            
        //connected to EPICS variables
        QELabel * tcLabel = new QELabel(varName, this);
        tcLabel->activate();
        layout->addWidget(tcLabel, i-1,1);
    }
    
    QLabel * CEC_label = new QLabel("CRISTCs:CEC_centre", this);
    layout->addWidget(CEC_label, 7,0);
    QELabel * tc_CEC = new QELabel("CRISTCs:CEC_centre", this);
    tc_CEC->activate();
    layout->addWidget(tc_CEC, 7,1);    
    
    widget->setFixedSize(300,400);
}