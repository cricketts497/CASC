#include "include/RemoteDataDevice.h"

RemoteDataDevice::RemoteDataDevice(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
RemoteDevice(deviceName, config, parent),
request_interval(200),
askDataTimer(new QTimer(this)),
file_mutex(file_mutex)
{
	if(device_failed)
		return;

	data_file = new QFile(file_path);
	QMutexLocker file_locker(file_mutex);
	data_file->resize(0);

	connect(askDataTimer, SIGNAL(timeout()), this, SLOT(askData()));
	connect(this, SIGNAL(device_fail()), askDataTimer, SLOT(stop()));
	askDataTimer->start(request_interval);
}

void RemoteDataDevice::askData()
{
	//check for other operations
	if(socket->state() != QAbstractSocket::UnconnectedState)
		return;
	
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

	//ask for the data
	QString c_string;
	QTextStream c(&c_string);
	c << "data_" << size;
	remoteCommand = c.readAll();

	socket->connectToHost(hostAddress, hostDevicePort);
	connection_timer->start();
	connect(socket, SIGNAL(readyRead()), this, SLOT(receiveData()));
}

void RemoteDataDevice::receiveData()
{
	QByteArray data = socket->readAll();
	socket->disconnectFromHost();
    
    if(data.endsWith(failMessage)){
        emit device_message(QString("REMOTE %1 ERROR: receiveData: fail message received from local").arg(device_name));
		emit device_fail();
		return;
    }

	if(data.endsWith(noDataMessage) || data.endsWith(okMessage))
		return;//up to date with local

	QMutexLocker file_locker(file_mutex);
	if(!data_file->open(QIODevice::Append)){
		emit device_message(QString("REMOTE %1 ERROR: receiveData: data_file->open(write)").arg(device_name));
		emit device_fail();
		return;
	}

	data_file->write(data);

	data_file->close();
}