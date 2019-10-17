#include "include/VacuumReadout.h"

VacuumReadout::VacuumReadout(CascConfig * config, QWidget * parent) :
CascWidget("Pressure", parent)
{
    QWidget * widget = new QWidget(this);
	setWidget(widget);
    
    QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);
    
    QStringList names = config->getDevice("VacuumNames");
    if(names.isEmpty()){
        return;
    }
    
    for(int i=1; i<names.length(); i++){
        const QString varName = QString("CRISVacuum:%1").arg(names.at(i));
        
        QLabel * varLabel = new QLabel(varName, this);
        layout->addWidget(varLabel, i-1,0);
            
        //connected to EPICS variables
        QELabel * vacuumLabel = new QELabel(varName, this);
        
        vacuumLabel->setNotationProperty(QELabel::Notations::Scientific);
        vacuumLabel->activate();
        
        layout->addWidget(vacuumLabel, i-1,1);
    }
       
    widget->setFixedSize(300,names.length()*50);
}