#ifndef EPICS_DEVICE_BUTTON
#define EPICS_DEVICE_BUTTON

#include "include/CascConfig.h"
#include "QELineEdit.h"

#include <QPushButton>
#include <QHBoxLayout>

const int MAX_N_STATUS_LABELS = 20;

class EpicsDeviceButton : public QWidget
{
    Q_OBJECT
public:
    EpicsDeviceButton(QString deviceName, QStringList statusWindows, QString start_tip, QString stop_tip, QString failTip, CascConfig * config, QWidget * parent = nullptr);
    
public slots:
    void deviceHasStarted();
    void deviceHasStopped();
    
    void device_status(QString status);
    
    void setFail();
    
signals:
    void toggle_device(bool on);
    
    void buttonMessage(QString message);
    
private slots:
    void toggleSetpoint();
    void toggleOn();
    void setButtonColour();
    
    void setpointTimeoutMessage();
    
private:
    const QString deviceName;

    QPushButton * button;
    QELineEdit * setpointLabel;
    QELineEdit * stateLabel;
    
    QELineEdit * statusLabels[MAX_N_STATUS_LABELS];
    
    const int nStatusWindows;
    QStringList compoundNames;
    
    const QColor offColour;
	const QColor onColour;
	const QColor failColour;
    
    const QString start_tip;
    const QString stop_tip;
    const QString fail_tip;
    
    QTimer * setpointTimer;
    const int setpointTimeout;
};

#endif // EPICS_DEVICE_BUTTON