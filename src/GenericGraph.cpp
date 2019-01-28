#include "include/GenericGraph.h"
#include <QtWidgets>

GenericGraph::GenericGraph(const QString tag_path, const QString pdl_path, QMainWindow *parent) :
QWidget(parent),
binWidth(2),
maxBinWidth(10000),
tag_pos(0),
tagger_started(false),
taggerUpdateTime(20),
pdl_pos(0),
pdl_started(false),
pdlUpdateTime(100),
binned_changed(false),
xStep(4),//as 4 axis divisions
yStep(4),
maxValueX(0),
maxValueY(0),
xAxisIndex(0),
yAxisIndex(0),
zoomed(false),
graphUpdateTime(45)
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
			if(binEdges.isEmpty()){
				binEdges.append(uint(time) - uint(time)%binWidth + binWidth);
				tag_times.append(time*packet_hits);
				counts.append(packet_hits);
				delts.append(time-lastPacketTime);
				pdl_wavenumbers.append(0);
				pdl_counts.append(0);
			}else if(time > binEdges.last()){
				while(time > binEdges.last()){
					binEdges.append(binEdges.last() + binWidth);
					tag_times.append(0);
					counts.append(0);
					delts.append(0);
					pdl_wavenumbers.append(0);
					pdl_counts.append(0);
				}
				tag_times.last() += time*packet_hits;
				counts.last() += packet_hits;
				delts.last() += (time-lastPacketTime);
			}else if(time < binEdges.first()){
				while(time < binEdges.first()){
					binEdges.prepend(binEdges.first() - binWidth);
					tag_times.prepend(0);
					counts.prepend(0);
					delts.prepend(0);
					pdl_wavenumbers.prepend(0);
					pdl_counts.prepend(0);
				}
				tag_times.first() += time*packet_hits;
				counts.first() += packet_hits;
				delts.first() += (time-lastPacketTime);
			}else{
				for(int i=binEdges.size()-1; i>=0; i--){
					if(time < binEdges.at(i)){
						tag_times[i] += time*packet_hits;
						counts[i] += packet_hits;
						delts[i] += (time-lastPacketTime);
					}
				}	
			}
		}

		lastPacketTime = time;
		tag_pos = tag_file->pos();

	}
	tag_file->close();

	if(tag_pos > cur_tag_pos)
		binned_changed = true;
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
		QString header;
		in >> header;
	}
	while(!pdl_file->atEnd()){
		in >> timestamp >> pdl_wavenumber;

		time = timestamp /2e9;

		if(binEdges.isEmpty()){
			binEdges.append(uint(time) - uint(time)%binWidth + binWidth);
			pdl_wavenumbers.append(pdl_wavenumber);
			pdl_counts.append(1);
			tag_times.append(0);
			counts.append(0);
			delts.append(0);
		}else if(time > binEdges.last()){
			while(time > binEdges.last()){
				binEdges.append(binEdges.last() + binWidth);
				pdl_wavenumbers.append(0);
				pdl_counts.append(0);
				tag_times.append(0);
				counts.append(0);
				delts.append(0);
			}
			pdl_wavenumbers.last() += pdl_wavenumber;
			pdl_counts.last()++;
		}else if(time < binEdges.first()){
			while(time < binEdges.first()){
				binEdges.prepend(binEdges.first() - binWidth);
				pdl_wavenumbers.prepend(0);
				pdl_counts.prepend(0);
				tag_times.prepend(0);
				counts.prepend(0);
				delts.prepend(0);
			}
			pdl_wavenumbers.first() += pdl_wavenumber;
			pdl_counts.first()++;
		}else{
			for(int i=binEdges.size()-1; i>=0; i--){
				if(time < binEdges.at(i)){
					pdl_wavenumbers[i] += pdl_wavenumber;
					pdl_counts[i]++;
				}
			}	
		}
	}
	pdl_file->close();

	pdl_pos = pdl_file->pos();
	if(pdl_pos > cur_pdl_pos)
		binned_changed = true;
}

void GenericGraph::updateGraph()
{
	if(!binned_changed || !tagger_started)
		return;

	series->clear();
	maxValueX = 0;
	maxValueY = 0;
	//time
	if(xAxisIndex == 0){
		//counts
		if(yAxisIndex == 0){
			for(int i=0; i<binEdges.size(); i++){
				if(counts.at(i) == 0)
					continue;
				qreal x = tag_times.at(i)/counts.at(i);
				qreal y = counts.at(i);

				if(x > maxValueX)
					maxValueX = x;
				if(y > maxValueY)
					maxValueY = y;

				series->append(x, y);
			}
		//rate
		}else if(yAxisIndex == 1){
			for(int i=0; i<binEdges.size(); i++){
				if(counts.at(i) == 0)
					continue;
				qreal x = tag_times.at(i)/counts.at(i);
				qreal y = counts.at(i)/delts.at(i);

				if(x > maxValueX)
					maxValueX = x;
				if(y > maxValueY)
					maxValueY = y;

				series->append(x, y);
			}
		}
	//pdl wavenumber
	}else if(xAxisIndex == 1){
		//counts
		if(yAxisIndex == 0){
			for(int i=0; i<binEdges.size(); i++){
				if(pdl_counts.at(i) == 0)
					continue;
				qreal x = pdl_wavenumbers.at(i)/pdl_counts.at(i);
				qreal y = counts.at(i);

				if(x > maxValueX)
					maxValueX = x;
				if(y > maxValueY)
					maxValueY = y;

				series->append(x, y);
			}
		//rate
		}else if(yAxisIndex == 1){
			for(int i=0; i<binEdges.size(); i++){
				if(pdl_counts.at(i) == 0)
					continue;
				qreal x = pdl_wavenumbers.at(i)/pdl_counts.at(i);
				qreal y = counts.at(i)/delts.at(i);

				if(x > maxValueX)
					maxValueX = x;
				if(y > maxValueY)
					maxValueY = y;

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
		xAxis->setMax(uint(maxValueX)- uint(maxValueX)%xStep +2*xStep);
		// xAxis->setMax(xAxis->max()+xStep);
		// axisX->setTickCount(axisX->max()+1/5+1);
	// }
	// if(maxValueY >= yAxis->max()){
		yAxis->setMax(uint(maxValueY*8/7) - uint(maxValueY*8/7)%yStep +2*yStep);
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
	binEdges.clear();
	if(tagger_started){
		tag_times.clear();
		counts.clear();
		delts.clear();

		//read all the tagger data
		tag_pos = 0;
		updateTag();
	}

	if(pdl_started){
		pdl_wavenumbers.clear();
		pdl_counts.clear();

		//read all the PDL data
		pdl_pos = 0;
		updatePdl();
	}

}

//When the parameter on the x-axis is changed using the combo box
void GenericGraph::changeXAxis(int newIndex)
{
	
	//time
	if(newIndex == 0){
		if(pdlUpdateTimer->isActive())
			pdlUpdateTimer->stop();
		xAxisIndex = newIndex;
		xAxis->setTitleText("Time / s");
	//pdl wavenumber
	}else if(newIndex == 1){
		if(pdl_started){
			if(!pdlUpdateTimer->isActive())
				pdlUpdateTimer->start(pdlUpdateTime);
			xAxisIndex = newIndex;
			xAxis->setTitleText("PDL Wavenumber / cm^-1");
		}else{
			xAxisCombo->setCurrentIndex(0);
		}
	}
	resetAxes();
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
	pdlUpdateTimer->start(pdlUpdateTime);
	pdl_started = true;
	changeBinWidth();
}

//when the reset axes push button is pressed
void GenericGraph::resetAxes()
{
	xAxis->setRange(0,xStep);
	yAxis->setRange(0,yStep);	

	zoomed = false;
	binned_changed = true;
	updateGraph();
}

