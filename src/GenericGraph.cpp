#include "include/GenericGraph.h"
#include <QtWidgets>

GenericGraph::GenericGraph(const QString tag_path, const QString pdl_path, QMainWindow *parent) :
QWidget(parent),
binWidth(2),
maxBinWidth(10000),
tag_pos(0),
tagger_started(false),
taggerUpdateTime(20),
lastPacketTime(0),
pdl_pos(0),
pdl_started(false),
pdlUpdateTime(100),
binned_changed(false),
xStep(4),//as 4 axis divisions
yStep(4),
// maxValueX(0),
// maxValueY(0),
xAxisIndex(0),
yAxisIndex(0),
zoomed(false),
graphUpdateTime(45),
start_time(0),
bindex(-1)
{
	//main layout
	layout = new QGridLayout(this);
	// setLayout(layout);

	//main chart
	chartView = new ZoomChartView(this);
	connect(chartView, SIGNAL(new_zoom(bool)), this, SLOT(chartZoomed(bool)));
	layout->addWidget(chartView,0,0);

	//graph formatting	
	series = new QScatterSeries(this);
	series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	series->setMarkerSize(15.0);
	chartView->chart()->addSeries(series);

	xAxis = new QValueAxis(this);
	xAxis->setTitleText("Time / s");
	xAxis->setRange(0,xStep);
	xAxis->setLabelFormat("%.3f");
	chartView->chart()->setAxisX(xAxis, series);

	yAxis = new QValueAxis(this);
	yAxis->setTitleText("Counts");
	yAxis->setRange(0,yStep);
	// yAxis->setRange(0,150);
	yAxis->setLabelFormat("%.3f");
	// yAxis->setTickCount(16);
	chartView->chart()->setAxisY(yAxis, series);

	//bin width edit box, a combo box to change the y-axis and a reset button for zoom
	QHBoxLayout *bottomLayout = new QHBoxLayout;

	binWidthLabel = new QLabel("Bin width / s:", this);
	binWidthEdit = new QSpinBox(this);
	binWidthEdit->setValue(int(binWidth));
	binWidthEdit->setRange(1,maxBinWidth);
	connect(binWidthEdit, SIGNAL(editingFinished()), this, SLOT(changeBinWidth()));

	QPushButton *resetButton = new QPushButton("Reset axes", this);
	connect(resetButton, SIGNAL(clicked()), this, SLOT(resetAxes()));

	QLabel *yAxisLabel = new QLabel("y:", this);
	yAxisCombo = new QComboBox(this);
	yAxisCombo->addItem("Tagger counts");
	yAxisCombo->addItem("Tagger rate / s^-1");
	connect(yAxisCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeYAxis(int)));

	QLabel *xAxisLabel = new QLabel("x:", this);
	xAxisCombo = new QComboBox(this);
	xAxisCombo->addItem("Time / s");
	xAxisCombo->addItem("PDL Wavenumber / cm^-1");
	connect(xAxisCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeXAxis(int)));

	bottomLayout->addWidget(resetButton);
	bottomLayout->addWidget(yAxisLabel);
	bottomLayout->addWidget(yAxisCombo);
	bottomLayout->addWidget(xAxisLabel);
	bottomLayout->addWidget(xAxisCombo);
	bottomLayout->addStretch();
	bottomLayout->addWidget(binWidthLabel);
	bottomLayout->addWidget(binWidthEdit);
	layout->addLayout(bottomLayout,1,0);

	tag_file = new QFile(tag_path, this);
	// need some way of checking for new packets?
	taggerUpdateTimer = new QTimer(this);
	connect(taggerUpdateTimer, SIGNAL(timeout()), this, SLOT(updateTag()));
	
	pdl_file = new QFile(pdl_path, this);
	pdlUpdateTimer = new QTimer(this);
	connect(pdlUpdateTimer, SIGNAL(timeout()), this, SLOT(updatePdl()));

	QTimer *graphUpdateTimer = new QTimer(this);
	connect(graphUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGraph()));
	graphUpdateTimer->start(graphUpdateTime);

	

	//debug
	// binEdges.append(0);
	// binned.append(QPointF(0,0));
	// binned_changed = true;
	// updateGraph();
}

void GenericGraph::appendZeros()
{
	tag_times.append(0);
	counts.append(0);
	delts.append(0);
	pdl_wavenumbers.append(0);
	pdl_counts.append(0);
}

void GenericGraph::prependZeros()
{
	tag_times.prepend(0);
	counts.prepend(0);
	delts.prepend(0);
	pdl_wavenumbers.prepend(0);
	pdl_counts.prepend(0);
}

void GenericGraph::clearAll()
{
	tag_times.clear();
	counts.clear();
	delts.clear();
	pdl_wavenumbers.clear();
	pdl_counts.clear();
}

//update graph with each new set of packets
// change so not linked to the tagger device? read a set of packets at a set rate?
void GenericGraph::updateTag()
{
	// if(!newPackets)
	// 	return;

	uint cur_tag_pos = tag_pos;

	if(!tag_file->open(QIODevice::ReadOnly)){
		qDebug() << "Couldn't open tagger file for reading";
		return;
	}
	
	qint64 timestamp;
	quint64 packet_hits;
	uchar flag;
	quint32 hit;
	qreal time;
	
	tag_file->seek(tag_pos);
	QDataStream in(tag_file);
	if(tag_pos == 0){
		qint64 header;
		in >> header;
		// start_time = header / 2e9;
		if(start_time < 1)
			start_time = qreal(header) / 1000;
		lastPacketTime = start_time;
	}
	while(!tag_file->atEnd()){
		//get the packet header
		in >> timestamp >> packet_hits >> flag;

		//get the hits
		for(uint i=0; i<packet_hits; i++){
			in >> hit;
		}

		//since Epoch? in units of 500ps
		// time = timestamp / 2e9;
		time = qreal(timestamp) / 1000;
		time -= start_time;

		if(xAxisIndex == 0)
			binTagger_byTime(time, packet_hits);
		else if(xAxisIndex == 1)
			binTagger_byPdl(time, packet_hits);
		
		lastPacketTime = time;
		// tag_pos = tag_file->pos();
	}
	tag_pos = tag_file->pos();
	tag_file->close();

	if(tag_pos > cur_tag_pos)
		binned_changed = true;
}

void GenericGraph::binTagger_byTime(qreal time, quint64 packet_hits)
{
	//new bin
	if(binEdges.isEmpty() || time >=binEdges[bindex].last() || time <binEdges[bindex].first()){
		QVector<qreal> edge(1, int(time) - int(time)%binWidth);
		edge.append(int(time) - int(time)%binWidth +binWidth);
		if(binEdges.contains(edge)){
			bindex = binEdges.indexOf(edge);
		}else{
			binEdges.append(edge);
			appendZeros();
			bindex = binEdges.size()-1;
		}
	}
	tag_times[bindex] += time*packet_hits;
	counts[bindex] += packet_hits;
	delts[bindex] += (time-lastPacketTime);
}


void GenericGraph::binTagger_byPdl(qreal time, quint64 packet_hits)
{
	if(binEdges.isEmpty()){
		//need pdl numbers to bin, do that first
		updatePdl();
	}else{
		int bin;
		for(int i=0; i<binEdges.size(); i++){
			for(int j=0; j<binEdges[i].size(); j+=2){
				if(time>=binEdges[i].at(j) && (time<binEdges[i].at(j+1) || j+1>=binEdges[i].size())){
					bin = i;
					break;
				}
			}

			if(bin)
				break;
		}

		if(bin){
			tag_times[bin] += time*packet_hits;
			counts[bin] += packet_hits;
			delts[bin] += (time-lastPacketTime);
		}
	}
}


void GenericGraph::updatePdl()
{
	uint cur_pdl_pos = pdl_pos;

	if(!pdl_file->open(QIODevice::ReadOnly)){
		qDebug() << "Couldn't open pdl file for reading";
		return;
	}

	quint64 timestamp;
	quint64 pdl_wavenumber;
	qreal time;

	pdl_file->seek(pdl_pos);
	QDataStream in(pdl_file);
	if(pdl_pos == 0){
		quint64 header;
		in >> header;
		if(start_time < 1)
			start_time = qreal(header)/1000;
	}
	while(!pdl_file->atEnd()){
		in >> timestamp >> pdl_wavenumber;

		//timestamp since epoch in ms
		time = qreal(timestamp)/1000;
		time -= start_time;

		if(xAxisIndex == 0)
			break;
		else if(xAxisIndex == 1)
			binPdl_byPdl(time, pdl_wavenumber);
	}
	pdl_file->close();

	pdl_pos = pdl_file->pos();
	if(pdl_pos > cur_pdl_pos)
		binned_changed = true;
}


void GenericGraph::binPdl_byPdl(qreal time, quint64 pdl_wavenumber)
{
	if(binEdges.isEmpty() || pdl_wavenumber >= binEdges_pdl[bindex]+binWidth || pdl_wavenumber < binEdges_pdl[bindex]){
		//stop time
		if(!binEdges.isEmpty()){
			binEdges[bindex].append(time);
		}
		//change bin
		uint pdl_wavenumber_edge = pdl_wavenumber - pdl_wavenumber%binWidth;
		if(binEdges_pdl.contains(pdl_wavenumber_edge)){
			bindex = binEdges_pdl.indexOf(pdl_wavenumber_edge);
			binEdges[bindex].append(time);
		}else{
			binEdges_pdl.append(pdl_wavenumber_edge);
			appendZeros();
			QVector<qreal> edge(1, time);
			binEdges.append(edge);
			bindex = binEdges_pdl.size()-1;
		}
	}
	pdl_wavenumbers[bindex] += pdl_wavenumber;
	pdl_counts[bindex]++;	
}


void GenericGraph::checkMinMax(qreal x, qreal y)
{
	if(x > maxValueX)
		maxValueX = x;
	if(x < minValueX)
		minValueX = x;
	if(y > maxValueY)
		maxValueY = y;
	// if(y < minValueY)
	// 	minValueY = y;
}

void GenericGraph::updateGraph()
{
	if(!binned_changed || !tagger_started)
		return;

	series->clear();
	maxValueX = 0;
	maxValueY = 0;
	minValueX = 1e10;
	// minValueY = 1e10;

	//time
	if(xAxisIndex == 0){
		//counts
		if(yAxisIndex == 0){
			for(int i=0; i<binEdges.size(); i++){
				if(counts.at(i) <= 0)
					continue;
				qreal x = tag_times.at(i)/counts.at(i);
				qreal y = counts.at(i);

				checkMinMax(x,y);

				series->append(x, y);
			}
		//rate
		}else if(yAxisIndex == 1){
			for(int i=0; i<binEdges.size(); i++){
				if(counts.at(i) <= 0 || delts.at(i) <= 0)
					continue;
				qreal x = tag_times.at(i)/counts.at(i);
				qreal y = counts.at(i)/delts.at(i);

				checkMinMax(x,y);

				series->append(x, y);
			}
		}
	//pdl wavenumber
	}else if(xAxisIndex == 1){
		//counts
		if(yAxisIndex == 0){
			for(int i=0; i<binEdges.size(); i++){
				if(pdl_counts.at(i) <= 0 || counts.at(i) <= 0)
					continue;
				qreal x = pdl_wavenumbers.at(i)/pdl_counts.at(i);
				qreal y = counts.at(i);

				checkMinMax(x,y);

				series->append(x, y);
			}
		//rate
		}else if(yAxisIndex == 1){
			for(int i=0; i<binEdges.size(); i++){
				if(pdl_counts.at(i) <= 0 || counts.at(i) <= 0 || delts.at(i) <=0)
					continue;
				qreal x = pdl_wavenumbers.at(i)/pdl_counts.at(i);
				qreal y = counts.at(i)/delts.at(i);

				checkMinMax(x,y);

				series->append(x, y);
			}
		}
	}

	//debug
	// binned.append(QPointF(holder,5));
	// holder++;
	// series->replace(binned);

	// if(maxValueX >= xAxis->max()){
		// xAxis->setMax(binned.last().x() - uint(binned.last().x())%xStep + 2*xStep);
	if(!zoomed){
		emit newEdge(uint(maxValueX)- uint(maxValueX)%xStep +xStep);
		// xAxis->setMax(uint(maxValueX)- uint(maxValueX)%xStep +2*xStep);
		// xAxis->setRange(uint(minValueX)-uint(minValueX)%xStep-xStep, uint(maxValueX)- uint(maxValueX)%xStep +xStep);
		xAxis->setRange(0, uint(maxValueX)- uint(maxValueX)%xStep +xStep);
		// xAxis->setMax(xAxis->max()+xStep);
		// axisX->setTickCount(axisX->max()+1/5+1);
	// }
	// if(maxValueY >= yAxis->max()){
		// yAxis->setMax(uint(maxValueY*8/7) - uint(maxValueY*8/7)%yStep +2*yStep);
		// yAxis->setRange(uint(minValueY)-uint(minValueY)%yStep, uint(maxValueY) - uint(maxValueY)%yStep +yStep);
		yAxis->setRange(0, uint(maxValueY*8/7) - uint(maxValueY*8/7)%yStep +yStep);
		// yAxis->setRange(0, maxValueY*8/7);
		// yAxis->setMax(yAxis->max()+yStep);
		// axisY->setTickCount(axisX->max()+1/5+1);
	}

	binned_changed = false;
}

void GenericGraph::chartZoomed(bool zoom)
{
	zoomed = zoom;
}

void GenericGraph::changeBinWidth()
{
	binWidth = binWidthEdit->value();
	//clear the current binned data
	bindex = -1;
	binEdges.clear();
	clearAll();
	start_time = 0;

	if(pdl_started){
		//read all the PDL data
		binEdges_pdl.clear();
		pdl_pos = 0;
		updatePdl();
	}

	if(tagger_started){
		//read all the tagger data
		tag_pos = 0;
		updateTag();
	}

	resetAxes();
}

//When the parameter on the x-axis is changed using the combo box
void GenericGraph::changeXAxis(int newIndex)
{
	//time
	if(newIndex == 0){
		xAxisIndex = newIndex;
		xAxis->setTitleText("Time / s");
		binWidthLabel->setText("Bin Width / s");
		changeBinWidth();
		if(pdlUpdateTimer->isActive())
			pdlUpdateTimer->stop();
	//pdl wavenumber
	}else if(newIndex == 1){
		if(pdl_started){
			xAxisIndex = newIndex;
			xAxis->setTitleText("PDL Wavenumber / cm^-1");
			binWidthLabel->setText("Bin Width / cm^-1");
			changeBinWidth();
			pdlUpdateTimer->start(pdlUpdateTime);
		}else{
			xAxisCombo->setCurrentIndex(0);
		}
	}
}

//When the parameter on the y-axis is changed using the combo box
void GenericGraph::changeYAxis(int newIndex)
{
	yAxisIndex = newIndex;
	//counts
	if(newIndex == 0){
		yAxis->setTitleText("Counts");
	//rate
	}else if(newIndex == 1){
		yAxis->setTitleText("Rate / s^-1");
	}
	resetAxes();
}

//When tagger device is started
void GenericGraph::newTagger()
{
	taggerUpdateTimer->start(taggerUpdateTime);
	tagger_started = true;
	changeBinWidth();
}

//when the pdl device is started
void GenericGraph::newPdl()
{
	// pdlUpdateTimer->start(pdlUpdateTime);
	pdl_started = true;
	changeBinWidth();
}

void GenericGraph::closedPdl()
{
	xAxisCombo->setCurrentIndex(0);
	pdl_started = false;
}

//when the reset axes push button is pressed
void GenericGraph::resetAxes()
{
	// xAxis->setRange(0,xStep);
	// yAxis->setRange(0,yStep);	

	zoomed = false;
	binned_changed = true;
	updateGraph();
}

