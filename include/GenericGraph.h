#ifndef GENERIC_GRAPH
#define GENERIC_GRAPH

#include "include/qcustomplot.h"

class GenericGraph : public QCustomPlot
{
	Q_OBJECT

public:
	GenericGraph(QMainWindow *parent);

private:
	QColor pen_colour = QColor(Qt::black);

};



#endif //GENERIC_GRAPH