#include "include/AgilisMirrorsWindow.h"

AgilisMirrorsWindow::AgilisMirrorsWindow(QString deviceName, uint nMirrors, QWidget * parent) :
CascWidget(deviceName, parent)
{
    QWidget * widget = new QWidget(this);
	setWidget(widget);
	
	QVBoxLayout * vlayout = new QVBoxLayout;
	widget->setLayout(vlayout);   

    QGridLayout * layout = new QGridLayout;
    
    for(int axis=0; axis<nMirrors*2; axis++){
        QELabel * approxPosition = new QELabel(QString("CASC:%1:Position%2").arg(deviceName).arg(axis+1));

        QEPushButton * buttons[4];
        for(int i=0; i<4; i++){
            buttons[i] = new QEPushButton(QString("CASC:%1:Jog%2").arg(deviceName).arg(axis+1), this);
            buttons[i]->setWriteOnPress(true);
            buttons[i]->setWriteOnRelease(true);
            buttons[i]->setReleaseText(QString("0"));
            buttons[i]->activate();
        }
        buttons[0]->setPressText(QString("4"));
        buttons[1]->setPressText(QString("1"));
        buttons[2]->setPressText(QString("-1"));
        buttons[3]->setPressText(QString("-4"));
        
        buttons[0]->setLabelTextProperty("Up fast");
        buttons[1]->setLabelTextProperty("Up slow");
        buttons[2]->setLabelTextProperty("Down slow");
        buttons[3]->setLabelTextProperty("Down fast");
        
        QEPushButton * calibrateButton = new QEPushButton(QString("CASC:%1:CalibrateCommanded").arg(deviceName), this);
        calibrateButton->setWriteOnClick(true);
        calibrateButton->setClickText(QString("%1").arg(axis+1));
        calibrateButton->setConfirmAction(true);
        calibrateButton->setConfirmText(QString("Confirm? Calibrating axis %1 on channel %2, axis will travel full +/- range and may not return to the same position, can take some time, ensure no laser is on the mirror").arg((axis+1)%2).arg(axis/2+1));
        calibrateButton->activate();
        
        layout->addWidget(calibrateButton, 0,axis);
        layout->addWidget(buttons[0], 1,axis);
        layout->addWidget(buttons[1], 2,axis);
        layout->addWidget(approxPosition, 3,axis);
        layout->addWidget(buttons[2], 4,axis);
        layout->addWidget(buttons[3], 5,axis);
    }
    
    vlayout->addLayout(layout);

    //wide stop button at the bottom
    QEPushButton * stopButton = new QEPushButton(QString("CASC:%1:StopCommanded").arg(deviceName), this);
    stopButton->setWriteOnClick(true);
    stopButton->setClickText("Stop");
    stopButton->activate();
    vlayout->addWidget(stopButton);
}