#ifndef CASC_WIDGET_H
#define CASC_WIDGET_H

#include <QDockWidget>
#include <QtWidgets>

class CascWidget : public QDockWidget
{
	Q_OBJECT

public:
	CascWidget(const char * name="Casc widget", QWidget * parent=nullptr);
	
protected:
	void closeEvent(QCloseEvent *event);
	
signals:
	void closing();
    
    void widget_message(QString message);
    void widget_fail();
};


#endif // CASC_WIDGET_H