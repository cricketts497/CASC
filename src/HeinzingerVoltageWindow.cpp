#include "include/HeinzingerVoltageWindow.h"

HeinzingerVoltageWindow::HeinzingerVoltageWindow(const QString voltage_file_path, QMutex * voltageFileMutex, uint maxVoltage, QWidget * parent) :
CascWidget("Heinzinger", parent),
voltageEdit(new QSpinBox(this)),
voltageSetButton(new DeviceButton("Set", this, "Set the voltage on the device", "", "SET FAIL")),
outputButton(new DeviceButton("Output off", this, "Turn on the output", "Turn off the output", "OUTPUT FAIL")),
voltageReadback(new QLabel("0", this)),
voltageFileMutex(voltageFileMutex)
{
	QWidget * widget = new QWidget(this);
	setWidget(widget);
	
	QVBoxLayout * vlayout = new QVBoxLayout;
	widget->setLayout(vlayout);
	
	voltageEdit->setValue(0);
	voltageEdit->setRange(0, maxVoltage);
    
    connect(voltageEdit, SIGNAL(valueChanged(int)), this, SLOT(voltageChanged()));
	connect(voltageSetButton, SIGNAL(toggle_device(bool)), this, SLOT(setVoltage(bool)));
    
    connect(outputButton, SIGNAL(toggle_device(bool)), this, SLOT(setOutput(bool)));
    connect(this, SIGNAL(widget_fail()), outputButton, SLOT(setFail()));
	
	QLabel * voltageEditLabel = new QLabel("V", this);
	QLabel * voltageReadbackLabel = new QLabel("Applied / V: ", this);
    
    QHBoxLayout * top = new QHBoxLayout;
	top->addWidget(voltageEdit, 1);
	top->addWidget(voltageEditLabel);
    top->addWidget(voltageSetButton);
    
    QHBoxLayout * bottom = new QHBoxLayout;
    bottom->addWidget(outputButton);
	bottom->addWidget(voltageReadbackLabel);
	bottom->addWidget(voltageReadback, 1);
    
    vlayout->addLayout(top);
    vlayout->addLayout(bottom);
	
	setFixedSize(400,150);
    
    voltage_file = new QFile(voltage_file_path, this);
}

void HeinzingerVoltageWindow::voltageChanged()
{
    //set the voltageSetButton to off
    if(voltageSetButton->started){
        voltageSetButton->toggle();
        voltageSetButton->setEnabled(true);
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
    }
}

void HeinzingerVoltageWindow::setOutput(bool start)
{
    if(start){
        outputButton->setText("Output on");
        emit sendCommand(QString("OUTP_1"));
    }else{
        outputButton->setText("Output off");
        emit sendCommand(QString("OUTP_0"));
    }
}

void HeinzingerVoltageWindow::readVoltage()
{
	bool locked = voltageFileMutex->tryLock();
    if(!locked)
        return;
    
    if(!voltage_file->open(QIODevice::ReadOnly)){
        emit widget_message("HEINZINGER VOLTAGE WINDOW ERROR: voltage_file->open(read)");
        emit widget_fail();
        voltageFileMutex->unlock();
        return;
    }
    
    //get the final voltage value, one quint64 => 8 bytes back from end
    voltage_file->seek(voltage_file->size()-8);
    QDataStream in(voltage_file);
    
    in >> current_applied_voltage;
    
    voltage_file->close();
    voltageFileMutex->unlock();
    
    voltageReadback->setText(QString::number(current_applied_voltage));
}