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
xAxisIndex(0),
yAxisIndex(0),
zoomed(false),
graphUpdateTime(45),
start_time(0),
bindex(-1),
// time_index(-1),
max_tof(50.0),
min_tof(0.0)
{
	//main layout
	layout = new QGridLayout(this);
	// setLayout(layout);

	//main chart
	chartView = new ZoomChartView(this);
	chartView->chart()->legend()->setVisible(false);
	connect(chartView, SIGNAL(new_zoom(bool)), this, SLOT(chartZoomed()));
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

}

void GenericGraph::appendZeros()
{
	tag_times.append(0);
	counts.append(0);
	delts.append(0);
	pdl_wavenumbers.append(0);
	pdl_counts.append(0);
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
	
	qint64 cur_tag_pos = tag_pos;

	if(!tag_file->open(QIODevice::ReadOnly)){
		emit graph_message(QString("GRAPH ERROR: tag_file->open()"));
		return;
	}
	
	qint64 timestamp;
	quint64 packet_hits;
	uchar flag;
	quint32 hit;
	qreal time;

	int hits;
	int offset;
	int max_tof_int = int(max_tof*2000);
	int min_tof_int = int(min_tof*2000);

	tag_file->seek(tag_pos);
	QDataStream in(tag_file);
	if(tag_pos == 0){
		qint64 header;
		in >> header;
		// start_time = header / 2e9;
		if(start_time < 1)
			start_time = qreal(header) / 1000;
	}
	while(!tag_file->atEnd()){
		//get the packet header
		in >> timestamp >> packet_hits >> flag;

		hits = int(packet_hits);

		//get the hits
		for(quint64 i=0; i<packet_hits; i++){
			in >> hit;
			offset = hit>>8&0xffffff;
			if(offset<min_tof_int || offset>max_tof_int)
				hits--;
		}

		if(hits <= 0){
			continue;
		}

		//since Epoch? in units of 500ps
		// time = timestamp / 2e9;
		time = qreal(timestamp) / 1000;
		time -= start_time;

		if(xAxisIndex == 0)
			binTagger_byTime(time, hits);
		else if(xAxisIndex == 1)
			binTagger_byPdl(time, hits);
		
		lastPacketTime = time;
		// tag_pos = tag_file->pos();
	}
	tag_pos = tag_file->pos();
	tag_file->close();

	if(tag_pos > cur_tag_pos){
		emit graph_message(QString("Graph: updateTag: position: %1").arg(tag_pos));
		binned_changed = true;
	}
}

void GenericGraph::binTagger_byTime(qreal time, int packet_hits)
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
	if(time-lastPacketTime>0){
		tag_times[bindex] += time*packet_hits;
		counts[bindex] += packet_hits;
		delts[bindex] += (time-lastPacketTime);
	}
}


void GenericGraph::binTagger_byPdl(qreal time, int packet_hits)
{
	// emit graph_message(QString("graph: tagger binning by pdl"));
	if(binEdges.isEmpty()){
		//need pdl numbers to bin, do that first
		updatePdl();
	}else if(!time_index || time_index+1>=binEdges[bindex].size() || time < binEdges[bindex].at(time_index) || time >= binEdges[bindex].at(time_index+1)){
		bool found_bin = false;
		int j;
		for(int i=0; i<binEdges.size(); i++){
			j=0;
			while(j<binEdges[i].size()){
				// emit graph_message(QString("graph: tagger: %1 %2").arg(binEdges[i][j]).arg(time));
				if(j+1>=binEdges[i].size()){
					if(time >= binEdges[i].at(j)){
						bindex = i;
						time_index = j;
						found_bin = true;
						break;
					}else
						break;
				}else if(time>=binEdges[i].at(j) && time<binEdges[i].at(j+1)){
					bindex = i;
					time_index = j;
					found_bin = true;
					break;
				}
				j+=2;
			}

			if(found_bin){
				emit graph_message(QString("Graph: tagger: bin %1, %2").arg(bindex).arg(time));
				tag_times[bindex] += time*packet_hits;
				counts[bindex] += packet_hits;
				delts[bindex] += (time-lastPacketTime);
				break;
			}else if(i>=binEdges.size()-1){
				emit graph_message(QString("Graph: tagger: no bin found"));
			}
		}
	}else{
		emit graph_message(QString("Graph: tagger: bin %1, %2").arg(bindex).arg(time));
		tag_times[bindex] += time*packet_hits;
		counts[bindex] += packet_hits;
		delts[bindex] += (time-lastPacketTime);
	}
}


void GenericGraph::updatePdl()
{
	// emit graph_message(QString("Graph: updatePdl: position: %1").arg(pdl_pos));
	if(xAxisIndex != 1)
		return;

	qint64 cur_pdl_pos = pdl_pos;

	if(!pdl_file->open(QIODevice::ReadOnly)){
		emit graph_message(QString("GRAPH ERROR: pdl_file->open()"));
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

	pdl_pos = pdl_file->pos();
	pdl_file->close();
	

	if(pdl_pos > cur_pdl_pos){
		emit graph_message(QString("Graph: updatePdl: position: %1").arg(pdl_pos));
		binned_changed = true;
	}
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
			emit graph_message(QString("Graph: PDL: change bin %1 %2").arg(bindex).arg(time));
			binEdges[bindex].append(time);
		}else{
			emit graph_message(QString("Graph: PDL: new bin %1 %2").arg(bindex).arg(time));
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
			// emit graph_message(QString("Graph: Switched to PDL binning"));
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
		// xAxis->setMax(uint(maxValueX)- uint(maxValueX)%xStep +2*xStep);
		// xAxis->setRange(uint(minValueX)-uint(minValueX)%xStep-xStep, uint(maxValueX)- uint(maxValueX)%xStep +xStep);
		// xAxis->setRange(uint(minValueX)-uint(minValueX)%xStep-xStep, uint(maxValueX)- uint(maxValueX)%xStep +xStep);
		// xAxis->setMax(xAxis->max()+xStep);
		// axisX->setTickCount(axisX->max()+1/5+1);
		xAxis->setRange(floor(minValueX/binWidth)*binWidth-binWidth, ceil(maxValueX/binWidth)*binWidth+binWidth);
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

void GenericGraph::chartZoomed()
{
	zoomed = true;
}

void GenericGraph::changeBinWidth()
{
	emit graph_message(QString("Graph: changeBinWidth"));
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
		// if(pdlUpdateTimer->isActive())
			// pdlUpdateTimer->stop();
	//pdl wavenumber
	}else if(newIndex == 1){
		if(pdl_started){
			xAxisIndex = newIndex;
			xAxis->setTitleText("PDL Wavenumber / cm^-1");
			binWidthLabel->setText("Bin Width / cm^-1");
			changeBinWidth();
			// pdlUpdateTimer->start(pdlUpdateTime);
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
	pdlUpdateTimer->start(pdlUpdateTime);
	pdl_started = true;
	changeBinWidth();
}

// void GenericGraph::closedPdl()
// {
// 	xAxisCombo->setCurrentIndex(0);
// 	pdl_started = false;
// }

//when the reset axes push button is pressed
void GenericGraph::resetAxes()
{
	xAxis->setRange(0,xStep);
	yAxis->setRange(0,yStep);	

	zoomed = false;
	binned_changed = true;
	updateGraph();
}

void GenericGraph::newSelectionWindow(qreal left, qreal right)
{
	max_tof = right;
	min_tof = left;
	changeBinWidth();
}

