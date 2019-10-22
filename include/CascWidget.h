#ifndef CASC_WIDGET_H
#define CASC_WIDGET_H

#include <QDockWidget>
#include <QtWidgets>

class CascWidget : public QDockWidget
{
	Q_OBJECT

public:
	CascWidget(QString name, QWidget * parent=nullptr);
	
protected:
	void closeEvent(QCloseEvent *event);
    
    const QColor offColour;
	const QColor onColour;
    const QColor failColour;
	
signals:
	void closing();
    
    void widget_message(QString message);
    void widget_fail();
    
    void sendCommand(QString command);
};


#endif // CASC_WIDGET_H