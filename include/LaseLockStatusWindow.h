#ifndef LASE_LOCK_STATUS_WINDOW
#define LASE_LOCK_STATUS_WINDOW

#include "QELabel.h"
#include "include/CascWidget.h"

class LaseLockStatusWindow : public CascWidget
{
    Q_OBJECT
public:
    LaseLockStatusWindow(QStringList laseLockStatusWindows, QWidget * parent = nullptr);
    
};

#endif // LASE_LOCK_STATUS_WINDOW