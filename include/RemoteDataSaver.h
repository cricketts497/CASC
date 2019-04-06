#ifndef REMOTE_DATA_SAVER_H
#define REMOTE_DATA_SAVER_H

#include "include/RemoteDevice.h"

class RemoteDataSaver : public RemoteDevice
{
    Q_OBJECT
public:
    RemoteDataSaver(bool heinzinger_start, CascConfig * config, QObject * parent=nullptr);   
    
};

#endif // REMOTE_DATA_SAVER_H