#include "include/GenericGraph.h"
#include <QtWidgets>

GenericGraph::GenericGraph(QMainWindow *parent) :
QCustomPlot(parent)
{
	QVector<double> x(101), y(101);
	for (int i=0; i<101; i++){
		x[i] = i/50.0 - 1;
		y[i] = x[i]*x[i];
	}
	
	addGraph();
	graph(0)->setPen(QPen(pen_colour));
	graph(0)->setData(x,y);
	xAxis->setRange(-1,1);
	yAxis->setRange(0,1);
	replot();

}