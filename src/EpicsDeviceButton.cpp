#include "include/EpicsDeviceButton.h"

EpicsDeviceButton::EpicsDeviceButton(QString deviceName, QStringList statusWindows, QString start_tip, QString stop_tip, QString fail_tip, CascConfig * config, QWidget * parent, QStringList commandWindows, QStringList urgentCommandWindows) : 
QWidget(parent),
deviceName(deviceName),
button(new QPushButton(this)),
setpointLabel(new QELineEdit(this)),
stateLabel(new QELineEdit(this)),
nStatusWindows(statusWindows.length()),
commandWindows(commandWindows),
urgentCommandWindows(urgentCommandWindows),
commandList(commandWindows),
urgentCommandList(urgentCommandWindows),
offColour(QColor(Qt::white)),
onColour(QColor(Qt::green)),
failColour(QColor(Qt::red)),
start_tip(start_tip),
stop_tip(stop_tip),
fail_tip(fail_tip),
setpointTimer(new QTimer(this)),
setpointTimeout(5000)
{
    if(nStatusWindows > MAX_N_STATUS_LABELS || commandWindows.length() > MAX_N_COMMAND_LABELS || urgentCommandWindows.length() > MAX_N_URGENT_COMMAND_LABELS){
        return;
    }
    
    //add the two widget to a layout
    QHBoxLayout * layout = new QHBoxLayout;
    setLayout(layout);
    
    layout->addWidget(button);
    layout->addWidget(setpointLabel);
    layout->addWidget(stateLabel);
    
    button->setAutoFillBackground(true);
    button->setText(deviceName);
    button->setStatusTip(start_tip);
    
    setpointLabel->setVariableName(QString("CASC:%1:SET").arg(deviceName),0);
    setpointLabel->activate();
    setpointLabel->setReadOnly(true);
    
    setpointLabel->setVisible(false);
    
    stateLabel->setVariableName(QString("CASC:%1:IS").arg(deviceName),0);
    stateLabel->activate();
    stateLabel->setReadOnly(true);
    
    stateLabel->setVisible(false);

    //device to widgets
    for(int i=0; i<nStatusWindows; i++){
        statusLabels[i] = new QELineEdit(QString("CASC:%1:%2").arg(deviceName).arg(statusWindows.at(i)), this);
        statusLabels[i]->activate();
        statusLabels[i]->setReadOnly(true);
        
        //add in user management here?
        // statusLabels[i]->setFixedWidth(40);
        // layout->addWidget(statusLabels[i]);
        statusLabels[i]->setVisible(false);
    }
    
    //widgets to devices
    for(int i=0; i<commandWindows.length(); i++){
        commandLabels[i] = new QELineEdit(QString("CASC:%1:%2").arg(deviceName).arg(commandWindows.at(i)), this);
        commandLabels[i]->activate();
        commandLabels[i]->setReadOnly(true);

        commandLabels[i]->setVisible(false);
        // layout->addWidget(commandLabels[i]);
        
        connect(commandLabels[i], SIGNAL(dbValueChanged()), this, SLOT(emitNewCommand()));
    }
    
    //widgets to devices
    for(int i=0; i<urgentCommandWindows.length(); i++){
        urgentCommandLabels[i] = new QELineEdit(QString("CASC:%1:%2").arg(deviceName).arg(urgentCommandWindows.at(i)), this);
        urgentCommandLabels[i]->activate();
        urgentCommandLabels[i]->setReadOnly(true);

        urgentCommandLabels[i]->setVisible(false);
        // layout->addWidget(commandLabels[i]);
        
        connect(urgentCommandLabels[i], SIGNAL(dbValueChanged()), this, SLOT(emitNewUrgentCommand()));
    }
    
    //change the setpoint on button click
    connect(button, SIGNAL(clicked()), this, SLOT(toggleSetpoint()));
    
    //stop trying to start/stop the device if not started/stopped within certain time
    setpointTimer->setInterval(setpointTimeout);
    setpointTimer->setSingleShot(true);
    connect(setpointTimer, SIGNAL(timeout()), this, SLOT(toggleSetpoint()));
    connect(setpointTimer, SIGNAL(timeout()), this, SLOT(setpointTimeoutMessage()));
    
    //emit a signal to turn the local device on setpoint PV change
    connect(setpointLabel, SIGNAL(dbValueChanged()), this, SLOT(toggleOn()));
    
    //show whether the device is on or off with the button colour
    setButtonColour();
    connect(stateLabel, SIGNAL(dbValueChanged()), this, SLOT(setButtonColour()));
        
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
        button->setEnabled(false);
        setpointTimer->start();
    }else if(setpointLabel->text() == "OFF" && (stateLabel->text() == "ON" || stateLabel->text() == "FAIL")){
        emit toggle_device(false);
        button->setEnabled(false);
        setpointTimer->start();
    }else{
        button->setEnabled(true);
        setpointTimer->stop();
    }
}

void EpicsDeviceButton::setButtonColour()
{
    button->setEnabled(true);
    setpointTimer->stop();
    
    QPalette pal = palette();
    if(stateLabel->text() == "FAIL"){
        pal.setColor(QPalette::Button, failColour);
        button->setFlat(true);
        button->setStatusTip(fail_tip);
    }else if(stateLabel->text() == "OFF"){
        pal.setColor(QPalette::Button, offColour);
        button->setFlat(false);
        button->setStatusTip(start_tip);
    }else if(stateLabel->text() == "ON"){
        pal.setColor(QPalette::Button, onColour);
        button->setFlat(true);
        button->setStatusTip(stop_tip);
    }
	button->setPalette(pal);
    button->update();
	update();
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

void EpicsDeviceButton::emitNewCommand()
{
    for(int i=0; i<commandWindows.length(); i++){
        if(commandLabels[i]->text() != commandList.at(i)){
            QString var_name = QString("CASC:%1:%2").arg(deviceName).arg(commandWindows.at(i));
            
            emit widgetCommand(QString("Command_%1_%2").arg(var_name).arg(commandLabels[i]->text()));
            
            commandList[i] = commandLabels[i]->text();
        }
    }
}

void EpicsDeviceButton::emitNewUrgentCommand()
{
    for(int i=0; i<urgentCommandWindows.length(); i++){
        if(urgentCommandLabels[i]->text() != urgentCommandList.at(i)){
            QString var_name = QString("CASC:%1:%2").arg(deviceName).arg(urgentCommandWindows.at(i));
            
            emit urgentWidgetCommand(QString("Command_%1_%2").arg(var_name).arg(urgentCommandLabels[i]->text()));
            
            urgentCommandList[i] = urgentCommandLabels[i]->text();
        }
    }
}

void EpicsDeviceButton::setpointTimeoutMessage()
{
    emit buttonMessage(QString("ERROR: Device %1 failed to start").arg(deviceName));
}



