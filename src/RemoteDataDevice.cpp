#include "include/RemoteDataDevice.h"

RemoteDataDevice::RemoteDataDevice(QString file_path, QMutex * file_mutex, QString deviceName, QString config_file_path, QObject * parent) :
RemoteDevice(deviceName, config_file_path, parent),
file_mutex(file_mutex)
{
	data_file = new QFile(file_path);

}

void RemoteDataDevice::getData()
{
	//ask for the data beyond
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

	QTextStream c;
	c << "data_" << size;

	QString command = c.readAll();

	//ask for the data
	sendCommand(command);

	//wait for the data to be sent
	if(!socket->waitForReadyRead(timeout)){
		emit device_message(QString("REMOTE %1 ERROR: getData: waitForReadyRead, %2: %3").arg(device_name).arg(hostName).arg(socket->errorString()));
		emit device_fail();
		return;
	}

	QByteArray data = socket->readAll();
	socket->disconnectFromHost();

	if(data.startsWith(char('n')))
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