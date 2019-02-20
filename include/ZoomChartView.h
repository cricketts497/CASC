#ifndef ZOOM_CHART_VIEW_H
#define ZOOM_CHART_VIEW_H

#include <QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

class ZoomChartView : public QChartView
{
	Q_OBJECT
public:
	ZoomChartView(QWidget *parent=nullptr);

signals:
	void new_zoom(bool zoom);

protected:
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif