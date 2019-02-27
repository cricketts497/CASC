#ifndef REMOTE_DATA_DEVICE_H
#define REMOTE_DATA_DEVICE_H

#include "include/RemoteDevice.h"

class RemoteDataDevice : public RemoteDevice
{
	Q_OBJECT
public:
	RemoteDataDevice(QString deviceName, QString config_file_path, QObject * parent=nullptr);

public slots:
	void getData();

private:
	QMutex * file_mutex;
	QFile * data_file;

}


#endif // REMOTE_DATA_DEVICE