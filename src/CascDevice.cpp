#include "include/CascDevice.h"

#include <QFile>

CascDevice::CascDevice(QString deviceName, QString config_file_path, QObject * parent) :
QObject(parent),
device_name(deviceName),
timeout(1000),
noDataMessage("no data")
{
	//load the config file
	QFile * configFile = new QFile(config_file_path);
	if(!configFile->open(QIODevice::ReadOnly)){
		emit device_message(QString("%1 ERROR: configFile->open(read)").arg(deviceName));
		emit device_fail();
		return;
	}
	
	QStringList device = nullptr;
	while(!configFile->atEnd()){
		QByteArray line = configFile->readLine();
		device = line.split(',');

		//line has structure: deviceName, hostName, hostListenPort, hostDevicePort
		if(device.first() == deviceName){
			break;
		}
	}
	configFile->close();
	if(!device || device.size() != 4){
		emit device_message(QString("%1 ERROR: device not found in config").arg(deviceName));
		emit device_fail();
		return;
	}

	hostName = device.at(1);
	hostListenPort = device.at(2).toUShort();
	hostDevicePort = device.at(3).toUShort();
}