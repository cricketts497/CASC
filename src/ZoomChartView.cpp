#include "include/ZoomChartView.h"
#include <QtGui/QMouseEvent>

ZoomChartView::ZoomChartView(QWidget *parent) :
QChartView(parent)
{
	setRubberBand(QChartView::RectangleRubberBand);
}

void ZoomChartView::mouseReleaseEvent(QMouseEvent *event)
{
	// emit zoom_message(QString("ZoomChartView: mouseReleaseEvent"));
	emit new_zoom(true);
	QChartView::mouseReleaseEvent(event);
}