#ifndef AGILIS_MIRRORS_H
#define AGILIS_MIRRORS_H

#include "include/SerialDevice.h"

const int AGILIS_MIRRORS_N_CHANNELS = 4;

class AgilisMirrors : public SerialDevice
{
    Q_OBJECT
public:
    AgilisMirrors(QString deviceName, CascConfig * config, QObject * parent=nullptr);
    
private slots:
    void mirrorCommand();
    void dealWithResponse(QByteArray resp);
    
private:
    void mirrorSingleWordCommand(QString command);
    void mirrorWidgetCommand(QStringList command_list);
    void mirrorTwoWordCommand(QStringList command_list);
    void mirrorThreeWordCommand(QStringList command_list);
    
    // void calibrate(uint axis);
    
    void responseErrorCode(QString response);
    void responseAxisStatus(QString response);
    void responseChannel(QString response);
    void responseLimitStatus(QString response);
    void responseNumSteps(QString response);

    const int travelRange;

    uint currentAxis;
    uint currentChannel;
    
    // bool calibrating;
    
    // uint stepRange[AGILIS_MIRRORS_N_CHANNELS*2];
    // bool axisLimitSwitches[AGILIS_MIRRORS_N_CHANNELS*2];
    int axisPos[AGILIS_MIRRORS_N_CHANNELS*2];
    
    int limitSign;
    
    // QString nextCalibrationCommand;
    
    bool moving;
};

#endif // AGILIS_MIRRORS_H