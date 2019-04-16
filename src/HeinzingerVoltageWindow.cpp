#include "include/HeinzingerVoltageWindow.h"

HeinzingerVoltageWindow::HeinzingerVoltageWindow(const QString voltage_file_path, QMutex * voltageFileMutex, uint maxVoltage, uint maxCurrent, QWidget * parent) :
CascWidget("Heinzinger", parent),
voltageEdit(new QSpinBox(this)),
voltageSetButton(new DeviceButton("Set volts", this, "Set the voltage on the device", "", "SET VOLTS FAIL")),
voltageReadback(new QLabel("0", this)),
currentEdit(new QSpinBox(this)),
currentSetButton(new DeviceButton("Set amps", this, "Set the current limit on the device", "", "SET AMPS FAIL")),
outputButton(new DeviceButton("Output off", this, "Turn on the output", "Turn off the output", "OUTPUT FAIL")),
voltageReadTimer(new QTimer(this)),
voltageReadTimeout(2399),
voltageFileMutex(voltageFileMutex)
{
	QWidget * widget = new QWidget(this);
	setWidget(widget);
	
	QVBoxLayout * vlayout = new QVBoxLayout;
	widget->setLayout(vlayout);
	
	voltageEdit->setValue(0);
	voltageEdit->setRange(0, maxVoltage);
    currentEdit->setValue(0);
    currentEdit->setRange(0, maxCurrent);
    
    voltageSetButton->setEnabled(false);
    connect(voltageEdit, SIGNAL(valueChanged(int)), this, SLOT(voltageChanged()));
	connect(voltageSetButton, SIGNAL(toggle_device(bool)), this, SLOT(setVoltage(bool)));
    
    currentSetButton->setEnabled(false);
    connect(currentEdit, SIGNAL(valueChanged(int)), this, SLOT(currentChanged()));
	connect(currentSetButton, SIGNAL(toggle_device(bool)), this, SLOT(setCurrent(bool)));
    
    outputButton->setEnabled(false);
    connect(outputButton, SIGNAL(toggle_device(bool)), this, SLOT(setOutput(bool)));
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
    if(on && !currentSetButton->started && !voltageSetButton->started){
        currentSetButton->setEnabled(true);
        voltageSetButton->setEnabled(true);
        
    }else if(!on){
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
        voltageSetButton->setEnabled(true);
    }
}

void HeinzingerVoltageWindow::currentChanged()
{
    //set the currentSetButton to off
    if(currentSetButton->started){
        currentSetButton->toggle();
        currentSetButton->setEnabled(true);
    }
}

void HeinzingerVoltageWindow::setVoltage(bool set)
{
    if(set){
        uint voltage = uint(voltageEdit->value());
        
        QString outString;
        QTextStream out(&outString);
        out << "VOLT_" << voltage;
        
        emit sendCommand(out.readAll());
        voltageSetButton->setEnabled(false);
        
        if(currentSetButton->started)
            outputButton->setEnabled(true);
    }
}

void HeinzingerVoltageWindow::setCurrent(bool set)
{
    if(set){
        uint current = uint(currentEdit->value());
        
        QString outString;
        QTextStream out(&outString);
        out << "CURR_" << current;
        
        emit sendCommand(out.readAll());
        currentSetButton->setEnabled(false);
        
        if(voltageSetButton->started)
            outputButton->setEnabled(true);
    }
}

void HeinzingerVoltageWindow::setOutput(bool start)
{
    if(start){
        outputButton->setText("Output on");
        emit sendCommand(QString("OUTP_1"));
        voltageReadTimer->start();
    }else{
        outputButton->setText("Output off");
        emit sendCommand(QString("OUTP_0"));
        voltageReadTimer->stop();
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
    
    if(voltage_file->size() > 16){
        //get the final voltage value, two quint64, one for integer, one for numbers after decimal point => 16 bytes back from end
        voltage_file->seek(voltage_file->size()-16);
        QDataStream in(voltage_file);
        in >> applied_voltage_int >> applied_voltage_decimal;
    }
    
    voltage_file->close();
    voltageFileMutex->unlock();
    
    voltageReadback->setText(QString::number(applied_voltage_int)+"."+QString::number(applied_voltage_decimal));
}