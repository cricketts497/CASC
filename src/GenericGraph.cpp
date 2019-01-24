#include "include/GenericGraph.h"
#include <QtWidgets>

GenericGraph::GenericGraph(const QString tag_path, QMainWindow *parent) :
QWidget(parent),
binWidth(2),
maxBinWidth(10000),
tag_path(tag_path),
tag_pos(0),
binned_changed(false),
timeStep(4),//as 4 axis divisions
countsStep(4),
maxValueX(0),
maxValueY(0),
graphUpdateTime(100),
yAxisIndex(0)
{
	//main layout
	layout = new QGridLayout(this);
	setLayout(layout);

	//main chart
	chartView = new QChartView(this);
	layout->addWidget(chartView,0,0);

	//graph formatting	
	series = new QScatterSeries(this);
	series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	series->setMarkerSize(15.0);
	chartView->chart()->addSeries(series);

	timeAxis = new QValueAxis(this);
	timeAxis->setTitleText("Time / s");
	timeAxis->setRange(0,timeStep);
	timeAxis->setLabelFormat("%.i");
	chartView->chart()->setAxisX(timeAxis, series);

	countsAxis = new QValueAxis(this);
	countsAxis->setTitleText("Counts");
	countsAxis->setRange(0,countsStep);
	countsAxis->setLabelFormat("%.i");
	// countsAxis->setTickCount(16);
	chartView->chart()->setAxisY(countsAxis, series);


	//bin width edit box and two combo boxes to change axes
	QHBoxLayout *bottomLayout = new QHBoxLayout(this);

	QLabel *binWidthLabel = new QLabel("Bin width / s:", this);
	binWidthEdit = new QSpinBox(this);
	binWidthEdit->setValue(binWidth);
	binWidthEdit->setRange(1,maxBinWidth);
	connect(binWidthEdit, SIGNAL(editingFinished()), this, SLOT(changeBinWidth()));

	QLabel *yAxisLabel = new QLabel("y:", this);
	yAxisCombo = new QComboBox(this);
	yAxisCombo->addItem("Tagger counts");
	yAxisCombo->addItem("Tagger rate / s^-1");
	connect(yAxisCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeYAxis(int)));

	bottomLayout->addWidget(yAxisLabel);
	bottomLayout->addWidget(yAxisCombo);
	bottomLayout->addStretch();
	bottomLayout->addWidget(binWidthLabel);
	bottomLayout->addWidget(binWidthEdit);
	layout->addLayout(bottomLayout,1,0);


	tag_file = new QFile(tag_path, this);

	QTimer *graphUpdateTimer = new QTimer(this);
	connect(graphUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGraph()));
	graphUpdateTimer->start(graphUpdateTime);

	//debug
	// binEdges.append(0);
	// binned.append(QPointF(0,0));
	// binned_changed = true;
	// updateGraph();
}

//update graph with each new set of packets
// change so not linked to the tagger device? read a set of packets at a set rate?
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
	qreal time;
	qreal del_t;
	QPointF last;

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
		time = timestamp / 2e9;

		//put into bin
		if(time > binEdges.last() || binned.isEmpty()){
			binEdges.append(time - uint(time)%binWidth + binWidth);
			if(yAxisIndex == 0)
				binned.append(QPointF(time,packet_hits));
			//ignore the first packet for rate
			else if(yAxisIndex == 1 && tag_pos!=0){
				del_t = time - lastPacketTime;
				binned.append(QPointF(time,packet_hits/del_t));
				counts.append(packet_hits);
			}
		}
		else{
			last = binned.last();
			if(yAxisIndex == 0){
				last.setX((last.x()*last.y()+time*packet_hits)/(last.y()+packet_hits));
				last.ry()+=packet_hits;
			}else if(yAxisIndex == 1){
				del_t = time - lastPacketTime;
				last.setX((last.x()*counts.last()+time*packet_hits)/(counts.last()+packet_hits));
				last.setY((counts.last()+packet_hits)/(counts.last()/last.y()+del_t));
				counts.last()+=packet_hits;
			}
			binned.replace(binned.size()-1, last);
		}

		if(binned.last().x() >= maxValueX)
			maxValueX = binned.last().x();
		if(binned.last().y() >= maxValueY)
			maxValueY = binned.last().y();

		lastPacketTime = time;
		tag_pos = tag_file->pos();

		// binned.append(QPointF(time, packet_hits));
		// binned.append(QPointF(holder, packet_hits));
		// holder+=1e4;
	}
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

	// if(maxValueX >= timeAxis->max()){
		// timeAxis->setMax(binned.last().x() - uint(binned.last().x())%timeStep + 2*timeStep);
	timeAxis->setMax(uint(maxValueX)- uint(maxValueX)%timeStep +2*timeStep);
		// timeAxis->setMax(timeAxis->max()+timeStep);
		// axisX->setTickCount(axisX->max()+1/5+1);
	// }
	// if(maxValueY >= countsAxis->max()){
	countsAxis->setMax(uint(maxValueY*8/7) - uint(maxValueY*8/7)%countsStep +2*countsStep);
		// countsAxis->setMax(countsAxis->max()+countsStep);
		// axisY->setTickCount(axisX->max()+1/5+1);
	// }

	binned_changed = false;
}

void GenericGraph::changeBinWidth()
{
	binWidth = binWidthEdit->value();

	//clear the current binned data
	binEdges.clear();
	binned.clear();

	maxValueX = 0;
	maxValueY = 0;

	//read all the tagger data
	tag_pos = 0;
	updateTag(true);
}

void GenericGraph::changeYAxis(int newIndex)
{
	yAxisIndex = newIndex;
	//counts
	if(newIndex == 0){
		countsAxis->setTitleText("Counts");
	//rate
	}else if(newIndex == 1){
		countsAxis->setTitleText("Rate / s^-1");
	}
	changeBinWidth();
}

//When tagger device is restarted
void GenericGraph::newTagger()
{
	changeBinWidth();
}