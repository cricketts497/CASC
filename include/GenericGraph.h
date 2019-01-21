#ifndef GENERIC_GRAPH
#define GENERIC_GRAPH

#include "include/qcustomplot.h"

class GenericGraph : public QCustomPlot
{
	Q_OBJECT

public:
	GenericGraph(uint binWidth, const QString tag_path, QMainWindow *parent);

private slots:
	void updateTag(bool newPackets);

private:
	QColor pen_colour = QColor(Qt::black);

	// //graph data
	uint binWidth;
	// QVector<double> *binEdges;
	// QVector<double> *counts;

	//tagger data
	const QString tag_path;
	QFile *tag_file;
	qint64 tag_pos;
};



#endif //GENERIC_GRAPH