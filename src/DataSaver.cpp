#include "include/DataSaver.h"

DataSaver::DataSaver(QStringList temp_file_path_list, QString final_file_folder, QVector<QMutex*> file_mutex_list, CascConfig * config, QObject * parent = nullptr) : 
CascDevice(QString("datadevice"), config, parent),
file_mutex_list(file_mutex_list)
{
    //create the QFile objects
    for(int i=0; i<temp_file_path_list; i++){
        temp_file_list.append(new QFile(temp_file_path_list.at(i)));
        
        QString final_path = final_file_folder;
        final_path.append("/scan_");
        final_path.append(QString::number(scan_number));
        final_path.append("/");
        final_path.append(temp_file_path_list.at(i).split("/").last().split("_").first());
        final_path.append(".csv");
        
        final_file_list.append(new QFile(final_path));
    }
    
}


void DataSaver::saveFinal()
{
    for(int i=0; i<temp_file_list; i++){
        QFile * temp_file = temp_file_list.at(i);
        QFile * final_file = final_file_list.at(i);
        QMutex * file_mutex = file_mutex_list.at(i);
        
        QMutexLocker fileLocker(file_mutex)
        if(!temp_file->open(QIODevice::ReadOnly)){
            emit device_message(QString("DATA SAVER ERROR: Unable to open temp file"))
            emit device_fail();
            return;
        }
        if(!final_file->open(QIODevice::WriteOnly | QIODevice::Text)){
            emit device_message(QString("DATA SAVER ERROR: Unable to open final file"))
            emit device_fail();
            return;
        }
        
        QDataStream from_temp(temp_file);
        QTextStream to_final(final_file);
                
        switch(i){
            //heinzinger file
            case 0:{
                qint64 timestamp;
                quint64 voltage_int;
                quint64 voltage_decimal;
                
                //header
                from_temp >> timestamp;
                while(!temp_file.atEnd()){
                    from_temp >> timestamp >> voltage_int >> voltage_decimal;
                    
                    to_final << timestamp << "," << voltage_int << "." << voltage_decimal << "\n";
                }
                
                break;
            }
        }
        temp_file->close();
        final_file->close();
    }
}