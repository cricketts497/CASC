#include "include/LaseLockStatusWindow.h"

LaseLockStatusWindow::LaseLockStatusWindow(QStringList laselockStatusWindows, QWidget * parent) : 
CascWidget(QString("LaseLock"),parent)
{
    QWidget * widget = new QWidget(this);
	setWidget(widget);
    
    QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);
    
    QLabel * regALab = new QLabel("Regulator A", this);
    QLabel * regBLab = new QLabel("Regulator B", this);
    layout->addWidget(regALab,0,0);
    layout->addWidget(regBLab,0,1);

    for(int i=0; i<4; i++){
        QELabel * regALabel = new QELabel(QString("CASC:Laselock:%1").arg(laselockStatusWindows.at(2*i)), this);
        QELabel * regBLabel = new QELabel(QString("CASC:Laselock:%1").arg(laselockStatusWindows.at(2*i+1)), this);
        
        regALabel->activate();
        regBLabel->activate();
        
        layout->addWidget(regALabel,i+1,0);
        layout->addWidget(regBLabel,i+1,1);
    }
    
    widget->setFixedSize(150,200);
}