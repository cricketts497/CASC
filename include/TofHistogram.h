#ifndef TOF_HISTOGRAM
#define TOF_HISTOGRAM

#include <QDockWidget>
#include <QtCharts>
#include "include/ZoomChartView.h"

class TofHistogram : public QDockWidget
{
	Q_OBJECT

public:
	TofHistogram(const QString tag_path, QWidget *parent = nullptr);

	void newTagger();

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void updateTag();
	void updateHist();

signals:
	void value(qreal x);
	void closing(bool x);

private:
	// const QString name = QString("TOF histogram");

	QLineSeries * line;
	QValueAxis * xAxis;
	QValueAxis * yAxis;
	int yStep;

	QFile * tag_file;
	qint64 tag_pos;
	QTimer * taggerUpdateTimer;
	int taggerUpdateTime;

	const int nBins = 10000;
	QVector<int> tofs;
	int binWidth;
	int maxBin;

	bool binned_changed;
	int graphUpdateTime;
};


#endif //TOF_HISTOGRAM