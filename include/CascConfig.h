#ifndef CASC_CONFIG_H
#define CASC_CONFIG_H

#include <QStringList>
#include <QVector>
#include <QFile>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QProcess>

class CascConfig : public QObject
{
	Q_OBJECT
public:
	CascConfig(QString config_file_path, QObject * parent=nullptr);

	QStringList getDevice(QString deviceName=QString());
	bool deviceLocal(QString deviceName);

signals:
	void config_message(QString message);

private:
	//device has structure: deviceName, hostName, hostListenPort, hostDevicePort
	QVector<QStringList> devices;
};

#endif // CASC_CONFIG_H