#ifndef DATA_SAVER_H
#define DATA_SAVER_H

// #include <QMutex>

#include "include/LocalDevice.h"

class DataSaver : public LocalDevice
{
    Q_OBJECT

public:
    DataSaver(QStringList temp_file_path_list, QString finalBasePath, QVector<QMutex*> file_mutex_list, CascConfig * config, QObject * parent = nullptr);

public slots:
    void scanStart();
    void scanStop();
    
private slots:
    void saverCommand(QString command);
    
private:
    uint getScanNumber();
    void setFilepaths(uint scan_number);
    
    QStringList temp_file_path_list;
    QVector<QFile*> temp_file_list;
    QVector<QMutex*> file_mutex_list;
    
    QVector<qint64> scan_start_positions;    
    
    QString finalBasePath;
    QVector<QFile*> final_file_list;
    
    bool scan_started;
    
    uint scan_number_offset;

};


#endif // DATA_SAVER_H