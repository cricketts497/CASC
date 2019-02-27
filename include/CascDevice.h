#ifndef CASC_DEVICE_H
#define CASC_DEVICE_H

#include <QObject>

class CascDevice : public QObject
{
	Q_OBJECT
public:
	CascDevice(QString deviceName, QString config_file_path, QObject * parent = nullptr);

signals:
	void device_message(QString message);
	void device_fail();

protected:
	const QString device_name;
	const int timeout;

	QString hostName;
	quint16 hostListenPort;
	quint16 hostDevicePort;
};

#endif // CASC_DEVICE_H