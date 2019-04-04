#ifndef HEINZINGER_PS_H
#define HEINZINGER_PS_H

#include "include/SerialDevice.h"

class HeinzingerPS : public SerialDevice
{
	Q_OBJECT
	
public:
	HeinzingerPS(uint voltage_limit, uint current_limit, QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent=nullptr);
    
public slots:
    void setVoltage(uint voltage);
    void setCurrent(uint current);
    void setOutput(bool on);
    
private slots:
    void dealWithResponse(QString response);
    void heinzingerCommand(QString command);
    void queryAppliedVoltage();
    
private:
    void setAverages(uint averages);

    void queryID();
    void queryVersion();
    void querySetVoltage();
    void querySetCurrent();
    void queryAppliedCurrent();
    void queryAverages(); 
    
    void responseID(QString response);
    void responseVersion(QString response);
    void responseSetVoltage(QString response);
    void responseSetCurrent(QString response);
    void responseAppliedVoltage(QString response);
    void responseAppliedCurrent(QString response);
    void responseAverages(QString response);
    
    QDateTime * time;
    QTimer * voltage_query_timer;
    const int voltage_query_timeout;
    
    int activeQuery;
    
    const uint voltage_limit;
    const uint current_limit;
    const QVector<uint> possible_averages = {1,2,4,8,16};
    
    uint voltage_setpoint;
    uint current_setpoint;
    uint averages_setpoint;
    
    bool volts_ok;
    bool amps_ok;
    
    uint voltage_set;
    uint current_set;
    quint64 voltage_applied;
    uint current_applied;
    uint averages_set;
};

#endif // HEINZINGER_PS_H