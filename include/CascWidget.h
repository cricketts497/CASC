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
	
};


#endif // CASC_WIDGET_H