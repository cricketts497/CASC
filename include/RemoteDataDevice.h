#ifndef REMOTE_DATA_DEVICE_H
#define REMOTE_DATA_DEVICE_H

#include "include/RemoteDevice.h"

#include <QTimer>

class RemoteDataDevice : public RemoteDevice
{
	Q_OBJECT
public:
	RemoteDataDevice(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent=nullptr);

private slots:
	void getData();

private:
	const int request_interval;
	QTimer * timer;

	QMutex * file_mutex;
	QFile * data_file;
};


#endif // REMOTE_DATA_DEVICE