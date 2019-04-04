#include "include/HeinzingerVoltageWindow.h"

HeinzingerVoltageWindow::HeinzingerVoltageWindow(const QString voltage_file_path, QMutex * voltageFileMutex, uint maxVoltage, QWidget * parent) :
CascWidget("Heinzinger", parent),
voltageEdit(new QSpinBox(this)),
voltageReadback(new QLabel("0", this)),
voltageFileMutex(voltageFileMutex)
{
	QWidget * widget = new QWidget(this);
	setWidget(widget);
	
	QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);
	
	voltageEdit->setValue(0);
	voltageEdit->setRange(0, maxVoltage);
	connect(voltageEdit, SIGNAL(editingFinished()), this, SLOT(setVoltage()));
	
	QLabel * voltageEditLabel = new QLabel("V", this);
	
	QLabel * voltageReadbackLabel = new QLabel("Readback / V: ", this);

	layout->addWidget(voltageEdit,0,0);
	layout->addWidget(voltageEditLabel,0,1);
	layout->addWidget(voltageReadbackLabel, 1,0);
	layout->addWidget(voltageReadback,1,1);
	
	setFixedSize(200,150);
    
    voltage_file = new QFile(voltage_file_path, this);
}

void HeinzingerVoltageWindow::setVoltage()
{
	uint voltage = uint(voltageEdit->value());
    
    QString outString;
	QTextStream out(&outString);
	out << "VOLT_" << voltage;
	
	emit sendCommand(out.readAll());
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