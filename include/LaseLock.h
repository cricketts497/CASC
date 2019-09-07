#ifndef LASE_LOCK_H
#define LASE_LOCK_H

#include "include/SerialDevice.h"

class LaseLock : public SerialDevice
{
    Q_OBJECT
public:
    LaseLock(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent = nullptr);
    
private slots:
    void lockCommand();
    void dealWithResponse(QByteArray response);
    
private:
    QString activeQuery;
    
    void responseRegLocked(QString response, bool regA);
    void checkBoolResponse(QString response, uint * flag);
    
    uint regALocked;
    uint regBLocked;
    
    uint regASearch;
    uint regBSearch;
    
    uint regARelock;
    uint regBRelock;
    
    uint regAHold;
    uint regBHold;
    
};

#endif // LASE_LOCK_H