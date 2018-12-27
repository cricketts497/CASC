#include "include/PdlDevice.h"
#include <QtWidgets>

PdlDevice::PdlDevice(int interval, QMainWindow *parent) :
QTimer(parent)
{
	value = 10000.0;

	connect(this, SIGNAL(timeout()), this, SLOT(increaseValue()));

	start(interval);

}

double PdlDevice::current_value()
{
	return value;
}

void PdlDevice::increaseValue()
{
	value += 1;
	emit newValue(value);
}