#ifndef GENERIC_GRAPH
#define GENERIC_GRAPH

#include <QtCharts>

class GenericGraph : public QWidget
{
	Q_OBJECT

public:
	GenericGraph(const QString tag_path, QMainWindow *parent);
	void newTagger();

private slots:
	void updateTag(bool newPackets);
	void updateGraph();
	void changeBinWidth();

private:
	//widget layout
	QGridLayout *layout;

	//graph data
	QChartView *chartView;
	QScatterSeries *series;
	QValueAxis *timeAxis;
	QValueAxis *countsAxis;

	//binWidth editing
	QSpinBox *binWidthEdit;
	const uint maxBinWidth;

	//graph updating
	uint graphUpdateTime;
	uint timeStep;
	uint countsStep;
	bool binned_changed;

	//binned data
	uint binWidth;
	QVector<quint64> binEdges;
	QVector<QPointF> binned;
	uint maxValueX;
	double maxValueY;

	//tagger data
	const QString tag_path;
	QFile *tag_file;
	qint64 tag_pos;

	//debug
	// int holder = 1e4;
};

#endif //GENERIC_GRAPH