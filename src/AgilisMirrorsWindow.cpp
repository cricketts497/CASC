#include "include/AgilisMirrorsWindow.h"

AgilisMirrorsWindow::AgilisMirrorsWindow(QString deviceName, uint nMirrors, QWidget * parent) :
CascWidget(name, parent)
{
    QWidget * widget = new QWidget(this);
	setWidget(widget);
	
	QVBoxLayout * vlayout = new QVBoxLayout;
	widget->setLayout(vlayout);   

    QGridLayout * layout = new QGridLayout;
    
    for(int mirror=0; mirror<nMirrors; mirror++){
        for(int axis=0; axis<2; axis++){
            // QESpinBox * relativeMove = new QESpinBox(QString("CASC:%1:"))
            
            
            
        }
    }
    
    vlayout->addLayout(layout);

    //wide stop button at the bottom
    QEPushButton * stopButton = new QEPushButton(QString("CASC:%1:StopCommanded").arg(deviceName), this);
    stopButton->setWriteOnClick(true);
    stopButton->setClickText("Stop");
    stopButton->activate();
    vlayout->addWidget(stopButton);
}