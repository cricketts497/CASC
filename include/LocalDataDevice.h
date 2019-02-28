#ifndef LOCAL_DATA_DEVICE_H
#define LOCAL_DATA_DEVICE_H

#include "include/LocalDevice.h"

class LocalDataDevice : public LocalDevice
{
	Q_OBJECT
public:
	LocalDataDevice(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent = nullptr);

private slots:
	void sendData(QString command, QTcpSocket * socket);

private:
	QFile * data_file;
	QMutex * file_mutex;

	const qint64 maxPayload;
};

#endif // LOCAL_DATA_DEVICE_H