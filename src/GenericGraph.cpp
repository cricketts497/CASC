#include "include/GenericGraph.h"
#include <QtWidgets>

GenericGraph::GenericGraph(uint bin_width, const QString tag_path, QMainWindow *parent) :
QChartView(new QChart(), parent),
binWidth(bin_width),
tag_path(tag_path),
tag_pos(0),
binned_changed(false),
timeStep(4),//as 4 axis divisions
countsStep(4),
graphUpdateTime(100)
{
	//graph formatting	
	series = new QScatterSeries();
	series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	series->setMarkerSize(15.0);
	chart()->addSeries(series);

	timeAxis = new QValueAxis;
	timeAxis->setTitleText("Time / s");
	timeAxis->setRange(0,timeStep);
	timeAxis->setLabelFormat("%.i");
	chart()->setAxisX(timeAxis, series);

	countsAxis = new QValueAxis;
	countsAxis->setTitleText("Counts");
	countsAxis->setRange(0,countsStep);
	countsAxis->setLabelFormat("%.i");
	// countsAxis->setTickCount(16);
	chart()->setAxisY(countsAxis, series);

	tag_file = new QFile(tag_path);

	QTimer *graphUpdateTimer = new QTimer();
	connect(graphUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGraph()));
	graphUpdateTimer->start(graphUpdateTime);

	//debug
	// binEdges.append(0);
	// binned.append(QPointF(0,0));
	// binned_changed = true;
	// updateGraph();
}

//update graph with each new packet
void GenericGraph::updateTag(bool newPackets)
{
	if(!newPackets)
		return;

	binned_changed = newPackets;

	if(!tag_file->open(QIODevice::ReadOnly)){
		qDebug() << "Couldn't open file for reading";
		return;
	}
	
	quint64 timestamp;
	quint64 packet_hits;
	uchar flag;

	tag_file->seek(tag_pos);
	QDataStream in(tag_file);
	if(tag_pos == 0){
		QString header;
		in >> header;
	}
	while(!tag_file->atEnd()){
		//get the packet header
		in >> timestamp >> packet_hits >> flag;

		//get the hits
		quint32 hit;
		for(uint i=0; i<packet_hits; i++){
			in >> hit;
		}

		//convert to s from units of 500ps
		qreal time = timestamp / 2e9;

		//put into bin
		if(time > binEdges.last() || binned.isEmpty()){
			binEdges.append(time - uint(time)%binWidth + binWidth);
			binned.append(QPointF(time,packet_hits));
		}
		else{
			QPointF last = binned.last();
			last.setX((last.x()*last.y()+time)/(last.y()+1));
			last.ry()+=packet_hits;
			binned.replace(binned.size()-1, last);
		}

		// binned.append(QPointF(time, packet_hits));
		// binned.append(QPointF(holder, packet_hits));
		// holder+=1e4;
	}

	tag_pos = tag_file->pos();
	tag_file->close();
}

void GenericGraph::updateGraph()
{
	if(!binned_changed)
		return;

	//debug
	// binned.append(QPointF(holder,5));
	// holder++;
	series->replace(binned);

	if(binned.last().x() >= timeAxis->max()){
		// timeAxis->setMax(binned.last().x() - uint(binned.last().x())%timeStep + 2*timeStep);
		timeAxis->setMax(uint(binned.last().x())- uint(binned.last().x())%timeStep +2*timeStep);
		// timeAxis->setMax(timeAxis->max()+timeStep);
		// axisX->setTickCount(axisX->max()+1/5+1);
	}
	if(binned.last().y() >= countsAxis->max()){
		countsAxis->setMax(uint(binned.last().y()*8/7) - uint(binned.last().y()*8/7)%countsStep +2*countsStep);
		// countsAxis->setMax(countsAxis->max()+countsStep);
		// axisY->setTickCount(axisX->max()+1/5+1);
	}

	binned_changed = false;
}