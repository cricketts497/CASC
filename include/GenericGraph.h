#ifndef GENERIC_GRAPH
#define GENERIC_GRAPH

#include "include/ZoomChartView.h"
#include <QtCharts>

class GenericGraph : public QWidget
{
	Q_OBJECT

public:
	GenericGraph(const QString tag_path, const QString pdl_path, QMainWindow *parent);
	void newTagger();
	void newPdl();

signals:
	void newEdge(qreal edge);

private slots:
	void updateTag();
	void updatePdl();
	void updateGraph();
	void changeBinWidth();
	void changeXAxis(int newIndex);
	void changeYAxis(int newIndex);
	void chartZoomed(bool zoom);
	void resetAxes();

private:
	//widget layout
	QGridLayout *layout;

	//graph data
	ZoomChartView *chartView;
	QScatterSeries *series;
	QValueAxis *xAxis;
	QValueAxis *yAxis;
	QValueAxis *rateAxis;

	//binWidth editing
	QSpinBox *binWidthEdit;
	const uint maxBinWidth;

	//axes editing
	QComboBox *yAxisCombo;
	uint yAxisIndex;
	QComboBox *xAxisCombo;
	uint xAxisIndex;

	//graph updating
	const uint graphUpdateTime;
	uint xStep;
	uint yStep;
	bool binned_changed;
	bool zoomed;

	//binned data
	int binWidth;
	QVector<int> binEdges;
	qreal start_time;
	void appendZeros();
	void prependZeros();
	void clearAll();

	void checkMinMax(qreal x, qreal y);
	qreal maxValueX;
	qreal maxValueY;
	qreal minValueX;
	qreal minValueY;
	qreal lastPacketTime;

	//tagger data
	QFile *tag_file;
	qint64 tag_pos;
	bool tagger_started;
	const uint taggerUpdateTime;
	QTimer *taggerUpdateTimer;

	QVector<qreal> tag_times;
	QVector<uint> counts;
	QVector<qreal> delts;

	//pdl data
	QFile *pdl_file;
	qint64 pdl_pos;
	bool pdl_started;
	const uint pdlUpdateTime;
	QTimer *pdlUpdateTimer;

	QVector<uint> pdl_wavenumbers;
	QVector<uint> pdl_counts;

	//debug
	// int holder = 1e4;
};

#endif //GENERIC_GRAPH