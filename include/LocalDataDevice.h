#ifndef LOCAL_DATA_DEVICE_H
#define LOCAL_DATA_DEVICE_H

#include "include/LocalDevice.h"

class LocalDataDevice : public LocalDevice
{
	Q_OBJECT
public:
	LocalDataDevice(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent = nullptr);

protected:
    QFile * data_file;
	QMutex * file_mutex;
    
private slots:
	void sendData(QString command);

private:
	const qint64 maxPayload;
};

#endif // LOCAL_DATA_DEVICE_H