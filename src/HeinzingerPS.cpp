#include "include/HeinzingerPS.h"

HeinzingerPS::HeinzingerPS(CascConfig * config, QObject * parent) :
SerialDevice(QString("heinzingerps"), config, parent),
voltage_setpoint(0),
true_voltage(0),
nAverages(4)
{
	//settings for serial communication with the power supplies taken from the manual
	serial_port->setBaudRate(QSerialPort::Baud9600);
	serial_port->setDataBits(QSerialPort::Data8);
	serial_port->setParity(QSerialPort::NoParity);
	serial_port->setStopBits(QSerialPort::OneStop);
	serial_port->setFlowControl(QSerialPort::HardwareControl);//HardwareControl or SoftwareControl	
	
	//need to get the port name, could have as argument in config file for device
	serial_port->setPortName(".....");
	
	serial_port->open(QIODevice::ReadWrite);	
}

void HeinzingerPS::setVoltage(uint voltage)
{
	voltage_setpoint = voltage;
	
	//reset the power supply
	serial_port->write("*RST\n");
	
	//set the voltage
	QString outString;
	QTextStream out(&outString);
	out << "VOLT " << voltage << "\n";
	serial_port->write(out.readAll().toUtf8());
	
	//query the voltage applied
	serial_port->write("VOLT?\n");
	connect(this, SIGNAL(newResponse(QString)), this, SLOT(applyVoltage(QString)));
}

void HeinzingerPS::applyVoltage(QString response)
{
	disconnect(this, SIGNAL(newReponse(QString)), 0,0);
	
	bool response_status;
	uint applied_voltage = response.toUInt(&response_status);
	if(!response_status){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Applied voltage response: %1").arg(response));
		emit device_fail();
		return;
	}
	
	if(applied_voltage != voltage_setpoint){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: Voltage applied different to setpoint: setpoint=%1 V, applied=%2 V").arg(voltage_setpoint).arg(applied_voltage));
		emit device_fail();
		return;
	}
	
	//turn on the output
	serial_port->write("OUTP ON\n");
	
	//set the number of recordings for average value calculation
	QString outString;
	QTextStream out(&outString);
	out << "AVER " << nAverages << "\n";
	serial_port->write(out.readAll().toUtf8());
	
	queryVoltage();
}

void HeinzingerPS::queryVoltage()
{
	//Query the effective output voltage
	serial_port->write("MEAS:VOLT?\n");
	connect(this, SIGNAL(newResponse(QString)), this, SLOT(readbackVoltage(QString)));	
}

void HeinzingerPS::readbackVoltage(QString response)
{
	disconnect(this, SIGNAL(newResponse(QString)),0,0);
	
	bool response_status;
	uint true_voltage_response = response.toUInt(&response_status);
	if(!response_status){
		emit device_message(QString("LOCAL SERIAL HEINZINGER ERROR: True voltage response: %1").arg(response));
		emit device_fail();
		return;
	}
	
	true_voltage = true_voltage_response;
}








