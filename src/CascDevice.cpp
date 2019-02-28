#include "include/CascDevice.h"

#include "include/CascConfig.h"

CascDevice::CascDevice(QString deviceName, CascConfig * config, QObject * parent) :
QObject(parent),
device_name(deviceName),
timeout(1000),
noDataMessage("no data"),
device_failed(false)
{
	messages.setString(&messages_string);

	QStringList device = config->getDevice(deviceName);
	if(device.isEmpty() || device.size() != 4){
		storeMessage(QString("%1 ERROR: device not found in config").arg(deviceName), true);
		return;
	}

	hostName = device.at(1);
	hostListenPort = device.at(2).toUShort();
	hostDevicePort = device.at(3).toUShort();
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