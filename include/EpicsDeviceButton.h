#ifndef EPICS_DEVICE_BUTTON
#define EPICS_DEVICE_BUTTON

#include "include/CascConfig.h"
#include "QELineEdit.h"

#include <QPushButton>
#include <QHBoxLayout>

const int MAX_N_STATUS_LABELS = 20;
const int MAX_N_COMMAND_LABELS = 3;

class EpicsDeviceButton : public QWidget
{
    Q_OBJECT
public:
    EpicsDeviceButton(QString deviceName, QStringList statusWindows, QString start_tip, QString stop_tip, QString failTip, CascConfig * config, QWidget * parent = nullptr, QStringList commandWindows = {});
    
public slots:
    void deviceHasStarted();
    void deviceHasStopped();
    
    void device_status(QString status);
    
    void setFail();
    
signals:
    void toggle_device(bool on);
    void widgetCommand(QString command);
    
    void buttonMessage(QString message);
    
private slots:
    void toggleSetpoint();
    void toggleOn();
    void setButtonColour();
    
    void emitNewCommand();
    
    void setpointTimeoutMessage();
    
private:
    const QString deviceName;

    QPushButton * button;
    QELineEdit * setpointLabel;
    QELineEdit * stateLabel;
    
    const int nStatusWindows;
    QStringList compoundNames;
    
    const QStringList commandWindows;
    QStringList commandList;
    
    const QColor offColour;
	const QColor onColour;
	const QColor failColour;
    
    const QString start_tip;
    const QString stop_tip;
    const QString fail_tip;
    
    QTimer * setpointTimer;
    const int setpointTimeout;

    QELineEdit * statusLabels[MAX_N_STATUS_LABELS];
    QELineEdit * commandLabels[MAX_N_COMMAND_LABELS];
};

#endif // EPICS_DEVICE_BUTTON