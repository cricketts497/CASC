#include "include/RemoteDataSaver.h"

RemoteDataSaver::RemoteDataSaver(bool heinzinger30k_start, CascConfig * config, QObject * parent) :
RemoteDevice(QString("datasaver"),config,parent)
{
    if(device_failed)
        return;
    
    if(heinzinger30k_start){
        startDevice("heinzingerps30k");
    }
}

/*!
Run whenever starting a device so data saver PC is always up to date with running devices.
*/
void RemoteDataSaver::startDevice(QString device_name)
{
    if(remoteDeviceCommandQueue.last() == QString("start_datasaver")){
        QString start_command = remoteDeviceCommandQueue.dequeue();
        start_command.append(QString("_%1").arg(device_name));
        remoteDeviceCommandQueue.enqueue(start_command);
    }else{
        QString outString;
        QTextStream out(&outString);
        out << "start_" << device_name;
        remoteDeviceCommand(out.readAll(), true);
    }
}