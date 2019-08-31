#include "include/HeinzingerVoltageWindow.h"

HeinzingerVoltageWindow::HeinzingerVoltageWindow(const QString voltage_file_path, QMutex * voltageFileMutex, uint maxVoltage, qreal maxCurrent, QWidget * parent) :
CascWidget(QString("Heinzinger %1kV").arg(maxVoltage/1000), parent),
voltageEdit(new QSpinBox(this)),
voltageSetButton(new DeviceButton("Set volts", this, "Set the voltage on the device", "", "SET VOLTS FAIL")),
voltageReadback(new QLabel("0", this)),
currentEdit(new QDoubleSpinBox(this)),
currentSetButton(new DeviceButton("Set amps", this, "Set the current limit on the device", "", "SET AMPS FAIL")),
outputButton(new DeviceButton("Output off", this, "Turn on the output", "Turn off the output", "OUTPUT FAIL")),
voltageReadTimer(new QTimer(this)),
voltageReadTimeout(500),
voltageFileMutex(voltageFileMutex)
{
	QWidget * widget = new QWidget(this);
	setWidget(widget);
	
	QVBoxLayout * vlayout = new QVBoxLayout;
	widget->setLayout(vlayout);
	
	voltageEdit->setValue(0);
	voltageEdit->setRange(0, maxVoltage);
    voltageEdit->setSingleStep(10);
    currentEdit->setValue(0.0);
    currentEdit->setRange(0.0, maxCurrent);
    currentEdit->setSingleStep(0.1);
    currentEdit->setDecimals(2);
    
    voltageSetButton->setEnabled(false);
    connect(voltageEdit, SIGNAL(valueChanged(int)), this, SLOT(voltageChanged()));
	connect(voltageSetButton, &QAbstractButton::clicked, this, &HeinzingerVoltageWindow::setVoltage);
    
    currentSetButton->setEnabled(false);
    connect(currentEdit, SIGNAL(valueChanged(double)), this, SLOT(currentChanged()));
	connect(currentSetButton, &QAbstractButton::clicked, this, &HeinzingerVoltageWindow::setCurrent);
    
    outputButton->setEnabled(false);
    connect(outputButton, &QAbstractButton::clicked, this, &HeinzingerVoltageWindow::setOutput);
    connect(this, SIGNAL(widget_fail()), outputButton, SLOT(setFail()));
	
	QLabel * voltageEditLabel = new QLabel("V", this);
	QLabel * voltageReadbackLabel = new QLabel("Applied / V: ", this);
    QLabel * currentEditLabel = new QLabel("mA", this);
    
    QHBoxLayout * top = new QHBoxLayout;
	top->addWidget(voltageEdit, 1);
	top->addWidget(voltageEditLabel);
    top->addWidget(voltageSetButton);
    
    QHBoxLayout * middle = new QHBoxLayout;
	middle->addWidget(currentEdit, 1);
	middle->addWidget(currentEditLabel);
    middle->addWidget(currentSetButton);
    
    QHBoxLayout * bottom = new QHBoxLayout;
    bottom->addWidget(outputButton);
	bottom->addWidget(voltageReadbackLabel);
	bottom->addWidget(voltageReadback, 1);
    
    vlayout->addLayout(top);
    vlayout->addLayout(middle);
    vlayout->addLayout(bottom);
	
	setFixedSize(400,250);
    
    voltage_file = new QFile(voltage_file_path, this);
    voltageReadTimer->setInterval(voltageReadTimeout);
    connect(voltageReadTimer, SIGNAL(timeout()), this, SLOT(readVoltage()));
}

void HeinzingerVoltageWindow::heinzingerDeviceOn(bool on)
{
    if(on){
        if(currentSetButton->started){
            currentSetButton->toggle();
        } 
        if(voltageSetButton->started){
            voltageSetButton->toggle();
        }        
        currentSetButton->setEnabled(true);
        voltageSetButton->setEnabled(true);
    }else{
        currentSetButton->setEnabled(false);
        voltageSetButton->setEnabled(false);
        outputButton->setEnabled(false);
    }
}

void HeinzingerVoltageWindow::voltageChanged()
{
    //set the voltageSetButton to off
    if(voltageSetButton->started){
        voltageSetButton->toggle();
        // voltageSetButton->setEnabled(true);
    }
}

void HeinzingerVoltageWindow::currentChanged()
{
    //set the currentSetButton to off
    if(currentSetButton->started){
        currentSetButton->toggle();
        // currentSetButton->setEnabled(true);
    }
}

void HeinzingerVoltageWindow::setVoltage()
{
    uint voltage = uint(voltageEdit->value());
    
    QString outString;
    QTextStream out(&outString);
    out << "VOLT_" << voltage;
    
    emit sendCommand(out.readAll());
    // voltageSetButton->setEnabled(false);
    
    if(currentSetButton->started)
        outputButton->setEnabled(true);
}

void HeinzingerVoltageWindow::setCurrent()
{
    qreal current = currentEdit->value();
    
    QString outString;
    QTextStream out(&outString);
    out << "CURR_" << current;
    
    emit sendCommand(out.readAll());
    // currentSetButton->setEnabled(false);
    
    if(voltageSetButton->started)
        outputButton->setEnabled(true);
}

void HeinzingerVoltageWindow::setOutput()
{
    if(!output_on){
        outputButton->setText("Output on");
        emit sendCommand(QString("OUTP_1"));
        voltageReadTimer->start();
        output_on = true;
    }else{
        outputButton->setText("Output off");
        emit sendCommand(QString("OUTP_0"));
        voltageReadTimer->stop();
        output_on = false;
    }
}

void HeinzingerVoltageWindow::readVoltage()
{
	bool locked = voltageFileMutex->tryLock();
    if(!locked)
        return;
    
    if(!voltage_file->open(QIODevice::ReadOnly)){
        emit widget_message("Heinzinger voltage window: unable to open voltage file");
        voltageFileMutex->unlock();
        return;
    }
    
    if(voltage_file->size() > 32){
        //get the final voltage value, two quint64, one for integer, one for numbers after decimal point => 16 bytes back from end
        voltage_file->seek(voltage_file->size()-16);
        QDataStream in(voltage_file);
        in >> applied_voltage_int >> applied_voltage_decimal;
    }
    
    voltage_file->close();
    voltageFileMutex->unlock();
    
    voltageReadback->setText(QString::number(applied_voltage_int)+"."+QString::number(applied_voltage_decimal));
}

void HeinzingerVoltageWindow::receiveHeinzingerStatus(QString status)
{
    //status, voltage setpoint, current setpoint, output setpoint
    QStringList status_list = status.split("_");
    
    //check for the correct format
    if(status_list.size() != 4 || status_list.first() != QString("Status"))
        return;
    
    // bool voltageSet = false;
    int voltage_setpoint = status_list.at(1).toInt();
    if(voltage_setpoint != voltageEdit->value() && !voltageEdit->hasFocus()){
        // voltageEdit->setValue(voltage_setpoint);
        // voltageSetButton->toggle();
        // voltageSet = true;
        voltageChanged();
    }else if(voltage_setpoint == voltageEdit->value() && !voltageSetButton->started && voltage_setpoint != 0){
        voltageSetButton->toggle();
    }
    
    qreal current_setpoint = status_list.at(2).toDouble();
    if(current_setpoint != currentEdit->value() && !currentEdit->hasFocus()){
        // currentEdit->setValue(current_setpoint);
        // currentSetButton->toggle();
        // if(voltageSet)
            // outputButton->setEnabled(true);
        currentChanged();
    }else if(current_setpoint == currentEdit->value() && !currentSetButton->started && current_setpoint !=0){
        currentSetButton->toggle();
    }
    
    if(status_list.at(3) == QString("1") && !outputButton->started){
        outputButton->toggle();
        outputButton->setText("Output on");
        voltageReadTimer->start();
        output_on = true;
    }else if(status_list.at(3) == QString("0") && outputButton->started){
        outputButton->toggle();
        outputButton->setText("Output off");
        voltageReadTimer->stop();
        output_on = false;
    }
}