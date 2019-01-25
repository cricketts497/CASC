#include "include/PanChartView.h"
#include <QtGui/QMouseEvent>

// PanChartView::PanChartView(QChart *chart, QWidget *parent) :
PanChartView::PanChartView(QWidget *parent) :
// QChartView(chart, parent),
QChartView(parent),
is_pressed(false)
{
	setRubberBand(QChartView::RectangleRubberBand);
}

//mouse press handling to pan
bool PanChartView::viewportEvent(QEvent *event)
{
	if(event->type() == QEvent::TouchBegin){
		is_pressed = true;
	}

	return QChartView::viewportEvent(event);
}

void PanChartView::mousePressEvent(QMouseEvent *event)
{
	if(is_pressed)
		return;
	QChartView::mousePressEvent(event);
}

void PanChartView::mouseMoveEvent(QMouseEvent *event)
{
	if(is_pressed)
		return;
	QChartView::mouseMoveEvent(event);
}

void PanChartView::mouseReleaseEvent(QMouseEvent *event)
{
	if(is_pressed)
		is_pressed = false;
	QChartView::mouseReleaseEvent(event);
}