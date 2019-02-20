#include "include/TofChartView.h"

TofChartView::TofChartView(QWidget * parent) :
ZoomChartView(parent),
zoom(true)
{

}

void TofChartView::mousePressEvent(QMouseEvent * event)
{
	if(event->modifiers().testFlag(Qt::ControlModifier)){
		setRubberBand(QChartView::HorizontalRubberBand);
		zoom = false;

		window_left = event->x();
	}
	QChartView::mousePressEvent(event);
}

void TofChartView::mouseReleaseEvent(QMouseEvent * event)
{
	if(zoom){
		ZoomChartView::mouseReleaseEvent(event);
	}else{
		setRubberBand(QChartView::NoRubberBand);
		QChartView::mouseReleaseEvent(event);
		setRubberBand(QChartView::RectangleRubberBand);
		zoom = true;

		window_right = event->x();
		QPointF scene_left = mapToScene(QPoint(window_left, 0));
		QPointF scene_right = mapToScene(QPoint(window_right, 0));
		qreal left = chart()->mapToValue(scene_left).x();
		qreal right = chart()->mapToValue(scene_right).x();
		emit selectionWindow(left, right);
	}
}
