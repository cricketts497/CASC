#ifndef LASE_LOCK_STATUS_WINDOW
#define LASE_LOCK_STATUS_WINDOW

#include "include/CascWidget.h"
#include "include/ParamReadout.h"

class LaseLockStatusWindow : public CascWidget
{
    Q_OBJECT
public:
    LaseLockStatusWindow(QWidget * parent = nullptr);

public slots:
    void receiveLaseLockStatus(QString status);
    
private:
    void setReadoutState(ParamReadout * readout, uint state);

    const QStringList onMessages;
    const QStringList offMessages;
    
    //off=0, on=1, fail=2
    const QList<uint> onStates;
    const QList<uint> offStates;
    
    ParamReadout * regASet[4];
    ParamReadout * regBSet[4];
};

#endif // LASE_LOCK_STATUS_WINDOW