#include "include/ZoomChartView.h"
#include <QtGui/QMouseEvent>

ZoomChartView::ZoomChartView(QWidget *parent) :
QChartView(parent)
{
	setRubberBand(QChartView::RectangleRubberBand);
}

void ZoomChartView::mouseReleaseEvent(QMouseEvent *event)
{
	emit new_zoom(true);
	QChartView::mouseReleaseEvent(event);
}