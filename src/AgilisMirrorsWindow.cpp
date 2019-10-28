#include "include/AgilisMirrorsWindow.h"

AgilisMirrorsWindow::AgilisMirrorsWindow(QString deviceName, int nMirrors, QWidget * parent) :
CascWidget(deviceName, parent)
{
    QWidget * widget = new QWidget(this);
	setWidget(widget); 

    QGridLayout * layout = new QGridLayout;
    widget->setLayout(layout);
    
    for(int axis=0; axis<nMirrors*2; axis++){
        if(axis%2 == 0){
            QLabel * channelLabel = new QLabel(QString("Channel %1").arg(axis/2+1), this);
            layout->addWidget(channelLabel,0,axis,1,2,Qt::AlignHCenter);
        }
        
        QLabel * axisLabel = new QLabel(QString("Axis %1").arg(axis%2+1), this);
        QELabel * approxPosition = new QELabel(QString("CASC:%1:Position%2").arg(deviceName).arg(axis+1), this);

        QEPushButton * buttons[4];
        for(int i=0; i<4; i++){
            buttons[i] = new QEPushButton(this);
            buttons[i]->setVariableNameProperty(QString("CASC:%1:Jog%2").arg(deviceName).arg(axis+1));
            buttons[i]->setWriteOnPress(true);
            buttons[i]->setWriteOnRelease(true);
            buttons[i]->setWriteOnClick(false);
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
        
        // QEPushButton * calibrateButton = new QEPushButton(this);
        // calibrateButton->setVariableNameProperty(QString("CASC:%1:ZeroCommanded").arg(deviceName));
        // calibrateButton->setWriteOnClick(true);
        // calibrateButton->setClickText(QString("%1").arg(axis+1));
        // calibrateButton->setConfirmAction(true);
        // calibrateButton->setConfirmText(QString("Confirm? Zeroing axis %1 on channel %2, axis will travel full +/- range, can take some time, ensure no laser is on the mirror").arg(axis%2+1).arg(axis/2+1));
        // calibrateButton->setLabelTextProperty("Zero");
        // calibrateButton->activate();
        
        layout->addWidget(axisLabel, 1,axis,Qt::AlignHCenter);
        // layout->addWidget(calibrateButton, 2,axis);
        layout->addWidget(buttons[0], 2,axis);
        layout->addWidget(buttons[1], 3,axis);
        layout->addWidget(approxPosition, 4,axis);
        layout->addWidget(buttons[2], 5,axis);
        layout->addWidget(buttons[3], 6,axis);
    }
    
    //wide stop button at the bottom
    QEPushButton * stopButton = new QEPushButton(this);
    stopButton->setVariableNameProperty(QString("CASC:%1:StopCommanded").arg(deviceName));
    stopButton->setWriteOnPress(true);
    stopButton->setWriteOnRelease(true);
    stopButton->setWriteOnClick(false);
    stopButton->setPressText("Stop");
    stopButton->setReleaseText("OK");
    stopButton->setLabelTextProperty("Stop");
    stopButton->activate();
    layout->addWidget(stopButton,8,0,1,nMirrors*2);
}