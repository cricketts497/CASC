#include "include/GenericGraph.h"
#include <QtWidgets>

GenericGraph::GenericGraph(const QString tag_path, QMainWindow *parent) :
QCustomPlot(parent),
tag_path(tag_path),
tag_pos(0)
{
	//graph formatting	
	addGraph();
	graph()->setPen(QPen(pen_colour));
	graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
	graph()->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsNone);

	tag_file = new QFile(tag_path);
}

//update graph with each new packet
void GenericGraph::updateTag(bool newPackets)
{
	if(!newPackets)
		return;

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

		//add the new packet to the graph
		graph()->addData(timestamp, packet_hits);
	}
	tag_pos = tag_file->pos();
	tag_file->close();

	graph()->rescaleAxes();
	replot();
}