#include "include/CascWidget.h"

CascWidget::CascWidget(QString name, QWidget * parent) :
QDockWidget(name, parent)
{
	
}

void CascWidget::closeEvent(QCloseEvent *event)
{
	emit closing();
}