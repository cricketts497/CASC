#ifndef TOF_CHART_VIEW
#define TOF_CHART_VIEW

#include "include/ZoomChartView.h"

class TofChartView : public ZoomChartView
{
	Q_OBJECT
public:
	TofChartView(QWidget * parent=nullptr);

protected:
	void mousePressEvent(QMouseEvent * event);
	void mouseDoubleClickEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);

signals:
	void selectionWindow(qreal left, qreal right);
	void chart_message(QString message);

private:
	int zoom;
	int window_left;
	int window_right;
};


#endif // TOF_CHART_VIEW