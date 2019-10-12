#include "include/HeinzingerVoltageWindow.h"

HeinzingerVoltageWindow::HeinzingerVoltageWindow(QString deviceName, uint maxVoltage, qreal maxCurrent, QWidget * parent) :
CascWidget(deviceName, parent),
offColour(QColor(Qt::white)),
onColour(QColor(Qt::green)),
outputSetpointTimer(new QTimer(this)),
outputSetpointTimeout(5000)
{
	QWidget * widget = new QWidget(this);
	setWidget(widget);
	
	QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);    

    QLabel * voltageEditLabel = new QLabel("Voltage:", this);
    QESpinBox * voltageEdit = new QESpinBox(this);
    QELabel * voltageSetpoint = new QELabel(QString("CASC:%1:VoltageSetpoint").arg(deviceName), this);

    QLabel * currentEditLabel = new QLabel("Current:", this);
    QESpinBox * currentEdit = new QESpinBox(this);
    QELabel * currentSetpoint = new QELabel(QString("CASC:%1:CurrentSetpoint").arg(deviceName), this);
    
    QLabel * outputEditLabel = new QLabel("Output:", this);
    outputButton = new QPushButton("Disconnected", this);
    outputCommanded = new QELineEdit(QString("CASC:%1:OutputCommanded").arg(deviceName), this);
    outputSetpoint = new QELineEdit(QString("CASC:%1:OutputSetpoint").arg(deviceName), this);
    QELabel * voltageApplied = new QELabel(QString("CASC:%1:VoltageApplied").arg(deviceName), this);
    
    layout->addWidget(voltageEditLabel,0,0);
    layout->addWidget(voltageEdit,0,1);
    layout->addWidget(voltageSetpoint,0,2);
    layout->addWidget(currentEditLabel,1,0);
    layout->addWidget(currentEdit,1,1);
    layout->addWidget(currentSetpoint,1,2);
    layout->addWidget(outputEditLabel,2,0);
    layout->addWidget(outputButton,2,1);
    layout->addWidget(voltageApplied,2,2);
    
    ////
    layout->addWidget(outputCommanded,2,3);
    layout->addWidget(outputSetpoint,2,4);
    ////
    
    voltageEdit->setVariableNameProperty(QString("CASC:%1:VoltageCommanded").arg(deviceName));
    voltageEdit->activate();
    voltageEdit->setAllowFocusUpdate(false);
    // voltageEdit->setWriteOnChange(true);
	voltageEdit->setRange(0, maxVoltage);
    voltageEdit->setSingleStep(10);
    voltageEdit->setDecimals(0);

    currentEdit->setVariableNameProperty(QString("CASC:%1:CurrentCommanded").arg(deviceName));
    currentEdit->activate();
    currentEdit->setAllowFocusUpdate(false);
    // currentEdit->setWriteOnChange(true);
    currentEdit->setRange(0.0, maxCurrent);
    currentEdit->setSingleStep(0.1);
    currentEdit->setDecimals(2);

    outputCommanded->setReadOnly(true);
    outputSetpoint->setReadOnly(true);
    // outputCommanded->setVisible(false);
    // outputSetpoint->setVisible(false);
    outputButton->setAutoFillBackground(true);
    setOutputButtonState();
    connect(outputSetpoint, SIGNAL(dbValueChanged()), this, SLOT(setOutputButtonState()));
    connect(outputButton, SIGNAL(clicked()), this, SLOT(toggleOutputCommanded()));
	
    //switch the output command if the setpoint doesn't change in a set time
    outputSetpointTimer->setInterval(outputSetpointTimeout);
    outputSetpointTimer->setSingleShot(true);
    connect(outputSetpointTimer, SIGNAL(timeout()), this, SLOT(toggleOutputCommanded()));
    connect(outputSetpointTimer, SIGNAL(timeout()), this, SLOT(outputSetpointTimeoutMessage()));
    
	// widget->setFixedSize(400,250);
}

void HeinzingerVoltageWindow::setOutputButtonState()
{
    outputSetpointTimer->stop();
    
    QPalette pal = palette();
    if(outputSetpoint->text() == "OutputOn"){
        outputButton->setText("Output on");
        outputButton->setEnabled(true);
        pal.setColor(QPalette::Button, onColour);
        outputButton->setFlat(true);
    }else if(outputSetpoint->text() == "OutputOff"){
        outputButton->setText("Output off");
        outputButton->setEnabled(true);
        pal.setColor(QPalette::Button, offColour);
        outputButton->setFlat(false);
    }else{
        outputButton->setText("Disconnected");
        outputButton->setEnabled(false);
        pal.setColor(QPalette::Button, offColour);
        outputButton->setFlat(false);
    }
    outputButton->setPalette(pal);
    outputButton->update();
    update();
}

void HeinzingerVoltageWindow::toggleOutputCommanded()
{
    //trying to change setpoint
    if(outputCommanded->text() == outputSetpoint->text()){
        outputButton->setText("Working...");
        outputButton->setEnabled(false);
        outputSetpointTimer->start();
    }else{
        setOutputButtonState();
    }
    
    if(outputCommanded->text() == "OutputOn"){
        outputCommanded->setText("OutputOff");
    }else if(outputCommanded->text() == "OutputOff"){
        outputCommanded->setText("OutputOn");
    }    
    outputCommanded->writeNow();
}

void HeinzingerVoltageWindow::outputSetpointTimeoutMessage()
{
    emit widget_message(QString("ERROR: Failed setting output"));
}