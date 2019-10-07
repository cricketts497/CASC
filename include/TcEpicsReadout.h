#ifndef TC_EPICS_READOUT
#define TC_EPICS_READOUT

#include "include/CascWidget.h"
#include "QELabel.h"

class TcEpicsReadout : public CascWidget
{
    Q_OBJECT
public:
    TcEpicsReadout(QWidget * parent = nullptr);

};


#endif // TC_EPICS_READOUT