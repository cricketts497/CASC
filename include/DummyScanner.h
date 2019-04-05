#ifndef DUMMY_SCANNER_H
#define DUMMY_SCANNER_H

#include "include/CascWidget.h"

class DummyScanner : public CascWidget
{
    Q_OBJECT
public:
    DummyScanner(QWidget * parent=nullptr);
    
private slots:
    void startClicked();
    void stopClicked();
    
private:
    QPushButton * startButton;
    QPushButton * stopButton;

};

#endif // DUMMY_SCANNER_H