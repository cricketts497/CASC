#include "include/RemoteDataSaver.h"

RemoteDataSaver::RemoteDataSaver(bool heinzinger_start, CascConfig * config, QObject * parent) :
RemoteDevice(QString("datasaver"),config,parent)
{
    if(heinzinger_start){
        remoteCommand = QString("start_heinzingerps");
        socket->connectToHost(hostAddress, hostListenPort);
        connection_timer->start();
    }
}