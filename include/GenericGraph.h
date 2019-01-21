#ifndef GENERIC_GRAPH
#define GENERIC_GRAPH

#include <QtCharts>

class GenericGraph : public QChartView
{
	Q_OBJECT

public:
	GenericGraph(uint binWidth, const QString tag_path, QMainWindow *parent);

private slots:
	void updateTag(bool newPackets);
	void updateGraph();

private:
	//graph data
	QScatterSeries *series;
	QValueAxis *timeAxis;
	QValueAxis *countsAxis;

	//graph updating
	uint graphUpdateTime;
	uint timeStep;
	uint countsStep;
	bool binned_changed;

	//binned data
	uint binWidth;
	QVector<quint64> binEdges;
	QVector<QPointF> binned;

	//tagger data
	const QString tag_path;
	QFile *tag_file;
	qint64 tag_pos;

	//debug
	// int holder = 1e4;
};

#endif //GENERIC_GRAPH