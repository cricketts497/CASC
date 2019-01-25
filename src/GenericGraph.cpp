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
yAxisIndex(0),
tagger_started(false),
zoomed(false)
{
	//main layout
	layout = new QGridLayout(this);
	// setLayout(layout);

	//main chart
	// PanChart *chart = new PanChart();
	chartView = new PanChartView(this);
	layout->addWidget(chartView,0,0);

	//graph formatting	
	series = new QScatterSeries(this);
	series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	series->setMarkerSize(15.0);
	chartView->chart()->addSeries(series);

	timeAxis = new QValueAxis(this);
	timeAxis->setTitleText("Time / s");
	timeAxis->setRange(0,timeStep);
	timeAxis->setLabelFormat("%.3f");
	chartView->chart()->setAxisX(timeAxis, series);

	countsAxis = new QValueAxis(this);
	countsAxis->setTitleText("Counts");
	countsAxis->setRange(0,countsStep);
	// countsAxis->setRange(0,150);
	countsAxis->setLabelFormat("%.3f");
	// countsAxis->setTickCount(16);
	chartView->chart()->setAxisY(countsAxis, series);

	//bin width edit box, a combo box to change the y-axis and a reset button for zoom
	QHBoxLayout *bottomLayout = new QHBoxLayout;

	QLabel *binWidthLabel = new QLabel("Bin width / s:", this);
	binWidthEdit = new QSpinBox(this);
	binWidthEdit->setValue(binWidth);
	binWidthEdit->setRange(1,maxBinWidth);
	connect(binWidthEdit, SIGNAL(editingFinished()), this, SLOT(changeBinWidth()));

	QPushButton *resetButton = new QPushButton("Reset axes", this);
	connect(resetButton, SIGNAL(clicked()), this, SLOT(resetAxes()));

	QLabel *yAxisLabel = new QLabel("y:", this);
	yAxisCombo = new QComboBox(this);
	yAxisCombo->addItem("Tagger counts");
	yAxisCombo->addItem("Tagger rate / s^-1");
	connect(yAxisCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeYAxis(int)));

	bottomLayout->addWidget(resetButton);
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
	quint32 hit;
	qreal time;
	
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
		for(uint i=0; i<packet_hits; i++){
			in >> hit;
		}

		time = timestamp / 2e9;

		//ignore the first packet for the rate calculations
		if(tag_pos!=0){
			if(time > binEdges.last() || binEdges.isEmpty()){
				binEdges.append(time - uint(time)%binWidth + binWidth);
				times.append(time*packet_hits);
				counts.append(packet_hits);
				delts.append(time-lastPacketTime);
			}else{
				times.last() += time*packet_hits;
				counts.last() += packet_hits;
				delts.last() += (time-lastPacketTime);
			}
		}

		lastPacketTime = time;
		tag_pos = tag_file->pos();

	}
	tag_file->close();
}

void GenericGraph::updateGraph()
{
	if(!binned_changed || !tagger_started)
		return;

	series->clear();
	maxValueX = 0;
	maxValueY = 0;
	if(yAxisIndex == 0){
		for(int i=0; i<times.size(); i++){
			qreal x = times.at(i)/counts.at(i);
			qreal y = counts.at(i);

			if(x > maxValueX)
				maxValueX = x;
			if(y > maxValueY)
				maxValueY = y;

			series->append(x, y);
		}
	}else if(yAxisIndex == 1){
		for(int i=0; i<times.size(); i++){
			qreal x = times.at(i)/counts.at(i);
			qreal y = counts.at(i)/delts.at(i);

			if(x > maxValueX)
				maxValueX = x;
			if(y > maxValueY)
				maxValueY = y;

			series->append(x, y);
		}
	}

	//debug
	// binned.append(QPointF(holder,5));
	// holder++;
	// series->replace(binned);

	// if(maxValueX >= timeAxis->max()){
		// timeAxis->setMax(binned.last().x() - uint(binned.last().x())%timeStep + 2*timeStep);
	if(!zoomed){
		timeAxis->setMax(uint(maxValueX)- uint(maxValueX)%timeStep +2*timeStep);
		// timeAxis->setMax(timeAxis->max()+timeStep);
		// axisX->setTickCount(axisX->max()+1/5+1);
	// }
	// if(maxValueY >= countsAxis->max()){
		countsAxis->setMax(uint(maxValueY*8/7) - uint(maxValueY*8/7)%countsStep +2*countsStep);
		// countsAxis->setMax(countsAxis->max()+countsStep);
		// axisY->setTickCount(axisX->max()+1/5+1);
	}

	binned_changed = false;
}

void GenericGraph::chartZoomed()
{
	zoomed = true;
}

void GenericGraph::changeBinWidth()
{
	binWidth = binWidthEdit->value();

	//clear the current binned data
	binEdges.clear();
	times.clear();
	counts.clear();
	delts.clear();

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
	binned_changed = true;
	updateGraph();
}

//When tagger device is started
void GenericGraph::newTagger()
{
	tagger_started = true;
	changeBinWidth();
}

void GenericGraph::resetAxes()
{
	timeAxis->setRange(0,timeStep);
	countsAxis->setRange(0,countsStep);	

	zoomed = false;
	binned_changed = true;
	updateGraph();
}

