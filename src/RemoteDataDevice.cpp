#include "include/RemoteDataDevice.h"

RemoteDataDevice::RemoteDataDevice(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
RemoteDevice(deviceName, config, parent),
request_interval(1000),
timer(new QTimer(this)),
file_mutex(file_mutex)
{
	if(device_failed)
		return;

	data_file = new QFile(file_path);
	QMutexLocker file_locker(file_mutex);
	data_file->resize(0);

	connect(timer, SIGNAL(timeout()), this, SLOT(getData()));
	connect(this, SIGNAL(device_fail()), timer, SLOT(stop()));
	timer->start(request_interval);
}

void RemoteDataDevice::getData()
{
	//ask for the data beyond end of current local file
	file_mutex->lock();
	if(!data_file->open(QIODevice::ReadOnly)){
		emit device_message(QString("REMOTE %1 ERROR: getData: data_file->open(Read)").arg(device_name));
		emit device_fail();
		file_mutex->unlock();
		return;
	}
	qint64 size = data_file->size();
	data_file->close();
	file_mutex->unlock();

	QString c_string;
	QTextStream c(&c_string);
	c << "data_" << size;

	QString command = c.readAll();

	//ask for the data
	if(!sendCommand(command))
		return;

	//wait for the data to be returned
	if(!socket->waitForReadyRead(timeout)){
		emit device_message(QString("REMOTE %1 ERROR: getData: waitForReadyRead, %2: %3").arg(device_name).arg(hostName).arg(socket->errorString()));
		emit device_fail();
		return;
	}

	QByteArray data = socket->readAll();
	socket->disconnectFromHost();

	if(data.endsWith(noDataMessage))
		return;//up to date with local

	QMutexLocker file_locker(file_mutex);
	if(!data_file->open(QIODevice::Append)){
		emit device_message(QString("REMOTE %1 ERROR: getData: data_file->open(write)").arg(device_name));
		emit device_fail();
		return;
	}

	data_file->write(data);

	data_file->close();
}