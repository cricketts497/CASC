#ifndef LOCAL_DATA_DEVICE_H
#define LOCAL_DATA_DEVICE_H

#include <QMutex>
#include <QDateTime>

#include "include/LocalDevice.h"

class LocalDataDevice : public LocalDevice
{
	Q_OBJECT
public:
	LocalDataDevice(const QStringList file_format, QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent = nullptr);

// private slots:
	// void sendData(QString command);

protected:
    void setStatus(QString status);
    
    bool saveToFile;

private:
    QFile * data_file;
	QMutex * file_mutex;
    
    QDateTime * time;

	// const qint64 maxPayload;
};

#endif // LOCAL_DATA_DEVICE_H