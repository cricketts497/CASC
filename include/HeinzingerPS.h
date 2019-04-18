#ifndef HEINZINGER_PS_H
#define HEINZINGER_PS_H

#include "include/SerialDevice.h"

class HeinzingerPS : public SerialDevice
{
	Q_OBJECT
	
public:
	HeinzingerPS(uint voltage_limit, uint current_limit, QString file_path, QMutex * file_mutex, QString deviceName, CascConfig * config, QObject * parent=nullptr);
 
signals:
    void voltage_set_zero();
 
public slots:
    void stop_device();
    
private slots:
    void dealWithResponse(QString response);
    void queryAfterSet();
    
    void heinzingerRemoteCommand(QString command);
   
    void queryAppliedVoltage();
 
private:
    void heinzingerCommand();

    void setVoltage(uint voltage);
    void setCurrent(qreal current);
    void setOutput(bool on);
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
    const qreal current_limit;
    const QVector<uint> possible_averages = {1,2,4,8,16};
    
    bool output_setpoint;
    
    int activeSetFunction;
    QTimer * queryAfterSetTimer;
    const int queryAfterSetTimeout;
    
    uint voltage_setpoint;
    qreal current_setpoint;
    uint averages_setpoint;
    
    bool volts_ok;
    bool amps_ok;
    
    uint voltage_set;
    qreal current_set;
    quint64 voltage_applied;
    quint64 voltage_decimal_applied;
    uint current_applied;
    uint current_decimal_applied;
    uint averages_set;
};

#endif // HEINZINGER_PS_H