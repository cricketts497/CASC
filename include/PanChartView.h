#ifndef PAN_CHART_VIEW_H
#define PAN_CHART_VIEW_H

#include <QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

class PanChartView : public QChartView
{
public:
	// PanChartView(QChart *chart, QWidget *parent=0);
	PanChartView(QWidget *parent=0);

protected:
	bool viewportEvent(QEvent *event);
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
	bool is_pressed;
};

#endif