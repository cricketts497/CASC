#include "include/FC0ServoWindow.h"

FC0ServoWindow::FC0ServoWindow(QString deviceName, QWidget * parent) :
CascWidget(deviceName, parent),
button(new QPushButton("Disconnected", this)),
stateSetpointTimer(new QTimer(this)),
stateSetpointTimeout(5000)
{
    setWidget(button);
    
    stateSetpoint = new QELineEdit(QString("CASC:%1:State").arg(deviceName), this);
    stateCommanded = new QELineEdit(QString("CASC:%1:StateCommanded").arg(deviceName), this);
    
    stateSetpoint->activate();
    stateCommanded->activate();
    
    stateSetpoint->setReadOnly(true);
    stateSetpoint->setVisible(false);
    stateCommanded->setReadOnly(true);
    stateCommanded->setVisible(false);
    
    button->setAutoFillBackground(true);
    setButtonState();
    connect(stateSetpoint, SIGNAL(dbValueChanged()), this, SLOT(setButtonState()));
    connect(button, SIGNAL(clicked()), this, SLOT(toggleStateCommanded()));  

    //switch the state command if the setpoint doesn't change in a set time
    stateSetpointTimer->setInterval(stateSetpointTimeout);
    stateSetpointTimer->setSingleShot(true);
    connect(stateSetpointTimer, SIGNAL(timeout()), this, SLOT(toggleStateCommanded()));
    connect(stateSetpointTimer, SIGNAL(timeout()), this, SLOT(stateSetpointTimeoutMessage()));
    
}

void FC0ServoWindow::setButtonState()
{
    stateSetpointTimer->stop();
    
    QPalette pal = palette();
    if(stateSetpoint->text() == "In"){
        button->setText("FC0 In");
        button->setEnabled(true);
        pal.setColor(QPalette::Button, onColour);
        button->setFlat(true);
    }else if(stateSetpoint->text() == "Out"){
        button->setText("FC0 Out");
        button->setEnabled(true);
        pal.setColor(QPalette::Button, offColour);
        button->setFlat(false);
    }else if(stateSetpoint->text() == "InvalidAngle"){
        button->setText("FC0 Unknown");
        button->setEnabled(true);
        pal.setColor(QPalette::Button, failColour);
        button->setFlat(true);    
    }else{
        button->setText("Disconnected");
        button->setEnabled(false);
    }
    button->setPalette(pal);
    button->update();
    update();
}

void FC0ServoWindow::toggleStateCommanded()
{
    //trying to change setpoint
    if(stateCommanded->text() == stateSetpoint->text()){
        button->setText("Working...");
        button->setEnabled(false);
        stateSetpointTimer->start();
    }else{
        setButtonState();
    }
    
    if(stateCommanded->text() == "In"){
        stateCommanded->setText("Out");
    }else if(stateCommanded->text() == "Out"){
        stateCommanded->setText("In");
    }    
    stateCommanded->writeNow();    
}

void FC0ServoWindow::stateSetpointTimeoutMessage()
{
    emit widget_message(QString("ERROR: Failed setting FC0 state"));
}