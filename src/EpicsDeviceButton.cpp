#include "include/EpicsDeviceButton.h"

EpicsDeviceButton::EpicsDeviceButton(QString deviceName, QStringList statusWindows, CascConfig * config, QWidget * parent) : 
QWidget(parent),
deviceName(deviceName),
button(new QPushButton(this)),
setpointLabel(new QELineEdit(this)),
stateLabel(new QELineEdit(this)),
nStatusWindows(statusWindows.length())
{
    if(nStatusWindows > MAX_N_STATUS_LABELS){
        return;
    }
    
    //add the two widget to a layout
    QHBoxLayout * layout = new QHBoxLayout;
    setLayout(layout);
    
    layout->addWidget(button);
    layout->addWidget(setpointLabel);
    layout->addWidget(stateLabel);
    
    button->setText(deviceName);
    
    setpointLabel->setVariableName(QString("CASC:%1:SET").arg(deviceName),0);
    setpointLabel->activate();
    setpointLabel->setReadOnly(true);
    
    stateLabel->setVariableName(QString("CASC:%1:IS").arg(deviceName),0);
    stateLabel->activate();
    stateLabel->setReadOnly(true);

    for(int i=0; i<nStatusWindows; i++){
        statusLabels[i] = new QELineEdit(QString("CASC:%1:%2").arg(deviceName).arg(statusWindows.at(i)), this);
        statusLabels[i]->activate();
        statusLabels[i]->setReadOnly(true);
        
        //add in user management here?
        statusLabels[i]->setFixedWidth(40);
        layout->addWidget(statusLabels[i]);
        // statusLabels[i]->setVisible(false);
    }
    
    //change the setpoint on button click
    connect(button, SIGNAL(clicked()), this, SLOT(toggleSetpoint()));
    
    //emit a signal to turn the local device on setpoint PV change
    connect(setpointLabel, SIGNAL(dbValueChanged()), this, SLOT(toggleOn()));
    
    //check for local devices
    bool local = config->deviceLocal(deviceName);
    
    //compounds of deviceName with first part before colon of each status window
    for(int i=0; i<nStatusWindows; i++){
        QString name = statusWindows.at(i).split(":").first();
        name.prepend(deviceName);
        if(!compoundNames.contains(name)){
            if(config->deviceLocal(name)){
                local =true;
            }
            compoundNames.append(name);
        }
    }
    
    //If local, the device can't be running on init
    if(local){
        setpointLabel->setText("OFF");
        setpointLabel->writeNow();
        stateLabel->setText("OFF");
        stateLabel->writeNow();
    }    
}

void EpicsDeviceButton::toggleSetpoint()
{    
    if(setpointLabel->text() == "ON"){
        setpointLabel->setText("OFF");
    }else if(setpointLabel->text() == "OFF"){
        setpointLabel->setText("ON");
    }
    setpointLabel->writeNow();
}

void EpicsDeviceButton::toggleOn()
{
    if(setpointLabel->text() == "ON" && stateLabel->text() == "OFF"){
        emit toggle_device(true);
    }else if(setpointLabel->text() == "OFF" && (stateLabel->text() == "ON" || stateLabel->text() == "FAIL")){
        emit toggle_device(false);
    }
}

void EpicsDeviceButton::deviceHasStarted()
{
    stateLabel->setText("ON");  
    stateLabel->writeNow();
}

void EpicsDeviceButton::deviceHasStopped()
{
    stateLabel->setText("OFF");
    stateLabel->writeNow();
}

void EpicsDeviceButton::setFail()
{
    stateLabel->setText("FAIL");
    stateLabel->writeNow();
}

void EpicsDeviceButton::device_status(QString status)
{
    QStringList status_list = status.split("_");
    
    // emit buttonMessage(status);
    
    if(status_list.first() != QString("Status") || status_list.length() < 2){
        return;
    }
    
    //single device for button
    if(status_list.at(1) == deviceName && status_list.length()-2 == nStatusWindows){
        //assign the values to the epics variables
        for(int i=2; i<status_list.length(); i++){
            statusLabels[i-2]->setText(status_list.at(i));
            statusLabels[i-2]->writeNow();
        }
        return;
    }
    
    //multiple sub-devices for a button
    int index = compoundNames.indexOf(status_list.at(1));//index of sub-device
    int subWindows = nStatusWindows/compoundNames.length();//number per sub device
    if(index >= 0 && status_list.length()-2 == subWindows){
        for(int i=2; i<status_list.length(); i++){
            statusLabels[index*subWindows+i-2]->setText(status_list.at(i));
            statusLabels[index*subWindows+i-2]->writeNow();
        }
    }
}



