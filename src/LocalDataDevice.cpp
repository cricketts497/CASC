#include "include/LocalDataDevice.h"

#include <QMutex>

LocalDataDevice::LocalDataDevice(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
LocalDevice(deviceName, config, parent),
file_mutex(file_mutex),
maxPayload(100000)
{
	if(device_failed)
		return;

	data_file = new QFile(file_path);

	connect(this, SIGNAL(newCommand(QString,QTcpSocket*)), this, SLOT(sendData(QString,QTcpSocket*)));
}

//send data command is QString("data_{position in file}")
void LocalDataDevice::sendData(QString command, QTcpSocket * socket)
{
	QStringList command_list = command.split("_");

	//check for send data command
	if(command_list.first() != QString("data") || command_list.size()<2)
		return;

	bool locked = file_mutex->tryLock();
	if(!locked){
		socket->write(noDataMessage);
		if(!socket->waitForDisconnected(timeout)){
			emit device_message(QString("LOCAL %1 ERROR: sendData: waitForDisconnected, %1: %2").arg(socket->peerName()).arg(socket->errorString()));
			emit device_fail();
		}
		return;
	}

	if(!data_file->open(QIODevice::ReadOnly)){
		emit device_message(QString("LOCAL %1 ERROR: sendData: data_file->open(read)"));
		emit device_fail();
		file_mutex->unlock();
		socket->write(noDataMessage);
		if(!socket->waitForDisconnected(timeout)){
			emit device_message(QString("LOCAL %1 ERROR: sendData: waitForDisconnected, %1: %2").arg(socket->peerName()).arg(socket->errorString()));
			emit device_fail();
		}
		return;
	}

	qint64 seek_pos = command_list.at(1).toLongLong();//returns 0 if conversion fails
	if(seek_pos >= data_file->size()){
		//remote is up to date
		data_file->close();
		file_mutex->unlock();
		
		socket->write(noDataMessage);
	}else{
		data_file->seek(seek_pos);

		QByteArray data = data_file->read(maxPayload);

		data_file->close();
		file_mutex->unlock();

		socket->write(data);
	}

	if(!socket->waitForDisconnected(timeout)){
		emit device_message(QString("LOCAL %1 ERROR: sendData: waitForDisconnected, %1: %2").arg(socket->peerName()).arg(socket->errorString()));
		emit device_fail();
	}
}
