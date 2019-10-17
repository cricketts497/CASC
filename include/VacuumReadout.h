#ifndef VACUUM_READOUT_H
#define VACUUM_READOUT_H

#include "include/CascWidget.h"
#include "QELabel.h"

#include "include/CascConfig.h"

class VacuumReadout : public CascWidget
{
    Q_OBJECT
public:
    VacuumReadout(CascConfig * config, QWidget * parent = nullptr);

};


#endif // VACUUM_READOUT_H