#include "include/CascWidget.h"

CascWidget::CascWidget(QString name, QWidget * parent) :
QDockWidget(name, parent),
offColour(QColor(Qt::white)),
onColour(QColor(Qt::green)),
failColour(QColor(Qt::red))
{
	
}

void CascWidget::closeEvent(QCloseEvent *event)
{
	emit closing();
}