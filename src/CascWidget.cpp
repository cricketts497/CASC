#include "include/CascWidget.h"

CascWidget::CascWidget(const char * name, QWidget * parent) :
QDockWidget(name, parent)
{
	
}

void CascWidget::closeEvent(QCloseEvent *event)
{
	emit closing();
}