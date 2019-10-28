#ifndef AGILIS_MIRRORS_WINDOW_H
#define AGILIS_MIRRORS_WINDOW_H

#include "include/CascWidget.h"

#include "QELabel.h"
#include "QEPushButton.h"

class AgilisMirrorsWindow : public CascWidget
{
    Q_OBJECT
public:
    AgilisMirrorsWindow(QString deviceName, int nMirrors, QWidget * parent=nullptr);

};

#endif // AGILIS_MIRRORS_WINDOW_H