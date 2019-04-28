#ifndef REMOTE_DATA_SAVER_H
#define REMOTE_DATA_SAVER_H

#include "include/RemoteDevice.h"

class RemoteDataSaver : public RemoteDevice
{
    Q_OBJECT
public:
    RemoteDataSaver(bool heinzinger30k_start, CascConfig * config, QObject * parent=nullptr); 

public slots:
    void startDevice(QString device);
    
};

#endif // REMOTE_DATA_SAVER_H