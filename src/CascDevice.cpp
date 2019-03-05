#include "include/CascDevice.h"

#include "include/CascConfig.h"

CascDevice::CascDevice(QString deviceName, CascConfig * config, QObject * parent) :
QObject(parent),
device_name(deviceName),
noDataMessage("no data"),
device_failed(false),
timeout(1000)
{
	connection_timer = new QTimer(this);
	connection_timer->setSingleShot(true);
	connection_timer->setInterval(timeout);
	
	messages.setString(&messages_string);
	
	QStringList listener = config->getDevice(QString("listener"));
	if(listener.isEmpty() || listener.size() != 2){
		storeMessage(QString("%1 ERROR: listener not found in config").arg(deviceName), true);
		return;
	}
	hostListenPort = listener.at(1).toUShort();

	QStringList device = config->getDevice(deviceName);
	if(device.isEmpty() || device.size() < 3){
		storeMessage(QString("%1 ERROR: device not found in config").arg(deviceName), true);
		return;
	}
	hostAddress = QHostAddress(device.at(1));
	hostDevicePort = device.at(2).toUShort();
	
	storeMessage(QString("%1: started").arg(deviceName), false);
}

CascDevice::~CascDevice()
{
	emit device_message(QString("%1: stopped").arg(device_name));
}

void CascDevice::stop_device()
{
	
}

void CascDevice::storeMessage(QString message, bool fail)
{
	messages << message;
	messages << endl;

	if(fail)
		device_failed = true;
}

void CascDevice::sendMessages()
{
	emit device_message(messages.readAll());
	if(device_failed)
		emit device_fail();
}