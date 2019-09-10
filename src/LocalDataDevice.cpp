#include "include/LocalDataDevice.h"

LocalDataDevice::LocalDataDevice(const QStringList file_format, QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent) :
LocalDevice(deviceName, config, parent),
saveToFile(false),
data_file(new QFile(file_path)),
file_mutex(file_mutex),
time(new QDateTime())
{
    //create the output file with a header
    if(!data_file->exists()){
        QMutexLocker file_locker(file_mutex);
        if(!data_file->open(QIODevice::WriteOnly | QIODevice::Text)){
            emit device_message(QString("LOCAL DATA DEVICE ERROR: %1: file->open(write)").arg(device_name));
            emit device_fail();
            return;
        }
        QTextStream out(data_file);
        out << "timestamp,device_name,";
        for(int i=0; i<file_format.length();i++){
            out<<file_format.at(i)<<",";
        }
        out<<endl;
        data_file->close();
    }
}

void LocalDataDevice::setStatus(QString status)
{
    QString old_status = getStatus();
    
    // update deviceStatus
    CascDevice::setStatus(status);

    //protected boolean controlled by top end devices
    if(!saveToFile){
        return;
    }

    QString new_status = getStatus();
    
    //if the status has changed, save it to the text file with a timestamp for the device type
    if(new_status != old_status){
        QStringList status_list = new_status.split("_");
        
        QMutexLocker file_locker(file_mutex);
        if(!data_file->open(QIODevice::Append | QIODevice::Text)){
            emit device_message(QString("LOCAL DATA DEVICE ERROR: %1: file->open(append)").arg(device_name));
            emit device_fail();
            return;
        }
        QTextStream out(data_file);
        
        qint64 timestamp = time->currentMSecsSinceEpoch();
        out << timestamp;
        out << ",";
        //don't store the 'Status' string
        for(int i=1; i<status_list.length(); i++){
            out << status_list.at(i) << ",";
        }
        out << endl;
        
        data_file->close();
    }
}

//only need data file on local machine as now using status
// //send data command is QString("data_{position in file}")
// void LocalDataDevice::sendData(QString command)
// {
	// QStringList command_list = command.split("_");

	// //check for send data command
	// if(command_list.first() != QString("data") || command_list.size()<2)
		// return;

	// bool locked = file_mutex->tryLock();
	// if(!locked){
		// socket->write(noDataMessage);
		// return;
	// }

	// if(!data_file->open(QIODevice::ReadOnly)){
		// emit device_message(QString("LOCAL %1 ERROR: sendData: data_file->open(read)"));
		// emit device_fail();
		// file_mutex->unlock();
		// socket->write(noDataMessage);
		// return;
	// }

	// qint64 seek_pos = command_list.at(1).toLongLong();//returns 0 if conversion fails
	// if(seek_pos >= data_file->size()){
		// //remote is up to date
		// data_file->close();
		// file_mutex->unlock();
		
		// socket->write(noDataMessage);
	// }else{
		// data_file->seek(seek_pos);

		// QByteArray data = data_file->read(maxPayload);

		// data_file->close();
		// file_mutex->unlock();

		// socket->write(data);
	// }
// }

