#include "include/DataSaver.h"

DataSaver::DataSaver(QStringList temp_file_path_list, QString finalBasePath, QVector<QMutex*> file_mutex_list, CascConfig * config, QObject * parent) : 
LocalDevice(QString("datasaver"), config, parent),
temp_file_path_list(temp_file_path_list),
file_mutex_list(file_mutex_list),
finalBasePath(finalBasePath),
scan_started(false)
{
    //!create the QFile objects
    for(int i=0; i<temp_file_path_list.length(); i++){
        temp_file_list.append(new QFile(temp_file_path_list.at(i)));
    }
    connect(this, SIGNAL(newCommand(QString)), this, SLOT(saverCommand(QString)));
    
    QStringList device = config->getDevice(QString("datasaver"));
	if(device.size() < 4){
		storeMessage(QString("DATA SAVER ERROR: Device not found in config"), true);
		return;
	}
	scan_number_offset = device.at(3).toUInt();
    
}

void DataSaver::saverCommand(QString command)
{
    if(command == QString("START") && !scan_started){
        scanStart();
    }else if(command == QString("STOP") && scan_started){
        scanStop();
    }
}

//record the position in the temp file where the scan started, i.e. the size of the file at the start of the scan
void DataSaver::scanStart()
{
    for(int i=0; i<temp_file_list.length(); i++){
        QFile * temp_file = temp_file_list.at(i);
        QMutex * file_mutex = file_mutex_list.at(i);
        
        QMutexLocker fileLocker(file_mutex);
        if(!temp_file->open(QIODevice::ReadOnly)){
            emit device_message(QString("DATA SAVER ERROR: Unable to open temp file"));
            emit device_fail();
            return;
        }
        scan_start_positions.append(temp_file->size());
        temp_file->close();        
    }
    emit device_message(QString("Local Data Saver: Scan started"));
    scan_started = true;
}
   
void DataSaver::scanStop()
{
    uint number = getScanNumber();
    
    QDir().mkdir(QString("%1/scan_%2").arg(finalBasePath).arg(number));
    
    setFilepaths(number);
    
    for(int i=0; i<temp_file_list.length(); i++){
        QFile * temp_file = temp_file_list.at(i);
        QFile * final_file = final_file_list.at(i);
        QMutex * file_mutex = file_mutex_list.at(i);
        qint64 start_pos = scan_start_positions.at(i);
        
        QMutexLocker fileLocker(file_mutex);
        if(!temp_file->open(QIODevice::ReadOnly)){
            emit device_message(QString("DATA SAVER ERROR: Unable to open temp file"));
            emit device_fail();
            return;
        }
        if(!final_file->open(QIODevice::WriteOnly | QIODevice::Text)){
            emit device_message(QString("DATA SAVER ERROR: Unable to open final file"));
            emit device_fail();
            return;
        }
        
        QDataStream from_temp(temp_file);
        QTextStream to_final(final_file);
        
        //position in the temp file at the start of the scan
        temp_file->seek(start_pos);
                
        switch(i){
            //heinzinger file
            case 0:{
                qint64 timestamp;
                quint64 voltage_int;
                quint64 voltage_decimal;
                
                //header
                if(start_pos == 0)
                    from_temp >> timestamp;
                while(!temp_file->atEnd()){
                    from_temp >> timestamp >> voltage_int >> voltage_decimal;
                    
                    to_final << timestamp << "," << voltage_int << "." << voltage_decimal << "\n";
                }
                
                break;
            }
        }
        temp_file->close();
        final_file->close();
    }
    emit device_message(QString("Local Data Saver: Scan stopped: %1").arg(number));
    scan_started = false;
}

uint DataSaver::getScanNumber()
{
    bool conv_ok;
    uint max_number = 0;
    QDirIterator it(finalBasePath);
    while(it.hasNext()){
        uint number = it.next().split("_").last().toUInt(&conv_ok);
        if(!conv_ok)
            continue;
        
        if(number > max_number){
            max_number = number;
        }   
    } 
    uint scan_number = max_number+1;
    
    if(scan_number == 1)
        scan_number += scan_number_offset;
    
    return scan_number;
}

void DataSaver::setFilepaths(uint scan_number)
{
    final_file_list.clear();
    //create the QFile objects
    for(int i=0; i<temp_file_path_list.length(); i++){         
        QString final_path = finalBasePath;
        final_path.append("/scan_");
        final_path.append(QString::number(scan_number));
        final_path.append("/");
        final_path.append(temp_file_path_list.at(i).split("/").last().split("_").first());
        final_path.append(".csv");
                
        final_file_list.append(new QFile(final_path));
    }
    
}


