#ifndef PARAM_READOUT_H
#define PARAM_READOUT_H

#include <QLineEdit>

class ParamReadout : public QLineEdit
{
    Q_OBJECT
public:
    ParamReadout(const char * text = "", QWidget * parent=nullptr);
    
public slots:
    void setOff();
    void setOK();
    void setFail();
      
private:    
    const QColor off_colour = QColor(Qt::white);
	const QColor ok_colour = QColor(Qt::green);
	const QColor fail_colour = QColor(Qt::red);
};


#endif // PARAM_READOUT_H