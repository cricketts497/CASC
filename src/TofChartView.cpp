#include "include/TofChartView.h"

TofChartView::TofChartView(QWidget * parent) :
ZoomChartView(parent),
zoom(0)
{

}

void TofChartView::mousePressEvent(QMouseEvent * event)
{
	// emit chart_message(QString("TOFView: mousePressEvent"));
	if(event->modifiers().testFlag(Qt::ControlModifier)){
		setRubberBand(QChartView::HorizontalRubberBand);
		zoom = 1;

		window_left = event->x();
	}
	QChartView::mousePressEvent(event);
}

void TofChartView::mouseDoubleClickEvent(QMouseEvent * event)
{
	// emit chart_message(QString("TOFView: mouseDoubleClickEvent"));
	zoom = 2;
	QChartView::mouseDoubleClickEvent(event);
}

void TofChartView::mouseReleaseEvent(QMouseEvent * event)
{
	// emit chart_message(QString("TOFView: mouseReleaseEvent"));
	if(zoom == 0){
		ZoomChartView::mouseReleaseEvent(event);
	}else if(zoom == 1){
		setRubberBand(QChartView::NoRubberBand);
		QChartView::mouseReleaseEvent(event);
		setRubberBand(QChartView::RectangleRubberBand);
		zoom = 0;

		window_right = event->x();

		if(window_left>window_right){
			int w = window_left;
			window_left = window_right;
			window_right = w;
		}

		QPointF scene_left = mapToScene(QPoint(window_left, 0));
		QPointF scene_right = mapToScene(QPoint(window_right, 0));
		qreal left = chart()->mapToValue(scene_left).x();
		qreal right = chart()->mapToValue(scene_right).x();
		emit selectionWindow(left, right);
	}else if(zoom == 2){
		setRubberBand(QChartView::NoRubberBand);
		QChartView::mouseReleaseEvent(event);
		emit new_zoom(false);
		setRubberBand(QChartView::RectangleRubberBand);
		zoom = 0;
	}
}
