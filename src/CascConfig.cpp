#include "include/CascConfig.h"

#include <QFile>
#include <QHostInfo>
#include <QNetworkInterface>

CascConfig::CascConfig(QString config_file_path, QObject * parent) :
QObject(parent)
{
	//load the config file
	QFile * configFile = new QFile(config_file_path);
	if(!configFile->open(QIODevice::ReadOnly)){
		emit config_message(QString("CONFIG ERROR: configFile->open(read)"));
		emit config_fail();
		return;
	}
	
	while(!configFile->atEnd()){
		QString line = QString(configFile->readLine());
		QStringList device = line.split(',');

		//device has structure: deviceName, hostAddress, hostListenPort, hostDevicePort
		devices.append(device);
	}
	configFile->close();

}

QStringList CascConfig::getDevice(QString deviceName)
{
	QStringList device;
	for(int i=0; i<devices.size(); i++){
		if(devices.at(i).first() == deviceName){
			device = devices.at(i);
		}
	}
	return device;
}

bool CascConfig::deviceLocal(QString deviceName)
{
	QStringList device = getDevice(deviceName);
	
	QHostAddress deviceAddress = QHostAddress(device.at(1));
	QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
	for(int i=0; i<ipAddressesList.size(); i++){
		if(ipAddressesList.at(i) == deviceAddress){
			return true;
		}
	}
	return false;
}