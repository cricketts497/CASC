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
		return;
	}
	
	while(!configFile->atEnd()){
		QString line = QString(configFile->readLine());
		QStringList device = line.split(',');

		//device has structure: deviceName,hostAddress,hostDevicePort
		//except for the listener which has structure: listener,listenerPort
		devices.append(device);
	}
	configFile->close();
}

QStringList CascConfig::getDevice(QString deviceName)
{
	QStringList device = {};
	for(int i=0; i<devices.size(); i++){
		if(devices.at(i).first() == deviceName){
			device = devices.at(i);
		}
	}
    
    if(device.isEmpty()){
        emit config_message(QString("CONFIG ERROR: device %1 not found").arg(deviceName));
    }
    
	return device;
}

bool CascConfig::deviceLocal(QString deviceName)
{
	if(deviceName == QString("listener"))
		return true;
	
	QStringList device = getDevice(deviceName);
	
    //device not in config
    if(device.isEmpty()){
        return true;
    }
    
	//debug config file, 5th argument for local/remote
	// //////////////////////
    if(device.length() >=5){
        if(device.at(4) == QString("local")){
            return true;
        }else if(device.at(4) == QString("remote")){
            return false;
        }
    }
	// //////////////////////

	//check local ip addresses
	QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
	for(int i=0; i<ipAddressesList.size(); i++){
		if(ipAddressesList.at(i).toString() == device.at(1)){
			return true;
		}
	}
	
	//check if local host name
	if(device.at(1) == QHostInfo::localHostName())
		return true;
	
	return false;
}



