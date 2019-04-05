#ifndef DATA_SAVER_H
#define DATA_SAVER_H

#include <QObject>

class DataSaver : public CascDevice
{
    Q_OBJECT

public:
    DataSaver(QStringList temp_file_path_list, QStringList final_file_path_list, CascConfig * config, QObject * parent = nullptr);

public slots:
    void saveFinal();
    
private:
    QVector<QFile*> temp_file_list;
    QVector<QFile*> final_file_list;
    
    QVector<QMutex*> 

};


#endif // DATA_SAVER_H