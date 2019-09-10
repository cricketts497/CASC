#ifndef LASE_LOCK_H
#define LASE_LOCK_H

#include "include/SerialDevice.h"

class LaseLock : public SerialDevice
{
    Q_OBJECT
public:
    LaseLock(QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent = nullptr);
    
private slots:
    void queryNextNumber();

    void lockCommand();
    void dealWithResponse(QByteArray response);
    
private:
    // QString getHexValues(QByteArray values);

    int queryNumber;
        
    const QStringList windows;
    QVector<uint> boolValues;
    
};

#endif // LASE_LOCK_H