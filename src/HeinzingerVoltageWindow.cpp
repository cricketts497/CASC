#include "include/HeinzingerVoltageWindow.h"

HeinzingerVoltageWindow::HeinzingerVoltageWindow(uint maxVoltage, QWidget * parent) :
CascWidget("Heinzinger", parent),
voltageEdit(new QSpinBox(this)),
voltageReadback(new QLabel("0", this))
{
	QWidget * widget = new QWidget(this);
	setWidget(widget);
	
	QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);
	
	voltageEdit->setValue(0);
	voltageEdit->setRange(0, maxVoltage);
	connect(voltageEdit, SIGNAL(editingFinished()), this, SLOT(changeVoltage()));
	
	QLabel * voltageEditLabel = new QLabel("V", this);
	
	QLabel * voltageReadbackLabel = new QLabel("Readback / V: ", this);

	layout->addWidget(voltageEdit,0,0);
	layout->addWidget(voltageEditLabel,0,1);
	layout->addWidget(voltageReadbackLabel, 1,0);
	layout->addWidget(voltageReadback,1,1);
	
	setFixedSize(200,150);
}

void HeinzingerVoltageWindow::changeVoltage()
{
	uint voltage = uint(voltageEdit->value());
	
	emit set_voltage(voltage);	
}

void HeinzingerVoltageWindow::readbackVoltage(qreal voltage)
{
	QString voltage_string = QString::number(voltage);
	
	voltageReadback->setText(voltage_string);
}