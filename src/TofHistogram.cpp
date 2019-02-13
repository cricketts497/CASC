#include "include/TofHistogram.h"
#include <QtWidgets>


TofHistogram::TofHistogram(const QString tag_path, QWidget * parent) :
QDockWidget("TOF Histogram", parent),
tag_pos(0),
binWidth(1*2000),
maxBin(0),
binned_changed(false),
graphUpdateTime(45),
taggerUpdateTime(20),
nBins(1000),
yStep(4)
{
	// setFloating(true);
	// setMinimumSize(300,300);
	setFixedSize(500,500);

	ZoomChartView * chartView = new ZoomChartView(this);
	line = new QLineSeries(this);
	chartView->chart()->addSeries(line);
	chartView->chart()->legend()->setVisible(false);
	setWidget(chartView);

	tofs = QVector<int>(nBins, 0);

	xAxis = new QValueAxis(this);
	xAxis->setTitleText("Time of flight / us");
	xAxis->setRange(-5,70);
	xAxis->setLabelFormat("%.3f");
	chartView->chart()->setAxisX(xAxis, line);

	yAxis = new QValueAxis(this);
	yAxis->setTitleText("Counts");
	yAxis->setRange(0,300);
	// yAxis->setRange(0,150);
	yAxis->setLabelFormat("%.3f");
	// yAxis->setTickCount(16);
	chartView->chart()->setAxisY(yAxis, line);

	tag_file = new QFile(tag_path, this);

	// need some way of checking for new packets?
	taggerUpdateTimer = new QTimer(this);
	connect(taggerUpdateTimer, SIGNAL(timeout()), this, SLOT(updateTag()));

	QTimer * graphUpdateTimer = new QTimer(this);
	connect(graphUpdateTimer, SIGNAL(timeout()), this, SLOT(updateHist()));
	graphUpdateTimer->start(graphUpdateTime);

}

void TofHistogram::updateTag()
{
	uint cur_tag_pos = tag_pos;

	if(!tag_file->open(QIODevice::ReadOnly)){
		qDebug() << "Couldn't open tagger file for reading";
		return;
	}
	
	qint64 timestamp;
	quint64 packet_hits;
	uchar flag;
	quint32 hit;
	int offset;

	tag_file->seek(tag_pos);
	QDataStream in(tag_file);
	if(tag_pos == 0){
		qint64 header;
		in >> header;
	}
	while(!tag_file->atEnd()){
		//get the packet header
		in >> timestamp >> packet_hits >> flag;

		//get the hits
		for(uint i=0; i<packet_hits; i++){
			in >> hit;
			offset = hit>>8&0xffffff;

			for(int i=binWidth; i<nBins*binWidth; i+=binWidth){
				if(offset<i){
					int bin = i/binWidth;

					if(bin>maxBin)
						maxBin = bin;

					tofs[bin]++;
					break;
				}
			}

		}
	}
	tag_pos = tag_file->pos();
	tag_file->close();

	if(tag_pos > cur_tag_pos)
		binned_changed = true;
}

void TofHistogram::updateHist()
{
	if(!binned_changed)
		return;

	line->clear();
	qreal maxValueY = 10;
	qreal minValueX = 0;
	qreal maxValueX = 50;

	qreal bw = binWidth/2000;

	for(int i=0; i<=maxBin; i++){
		if(tofs[i] < 0)
			continue;

		qreal x = qreal(i*binWidth/2000);
		qreal x_p = qreal((i*binWidth+binWidth)/2000);
		
		if(x < minValueX && qreal(tofs[i]) > 0)
			minValueX = x-bw;
		if(x_p+bw>maxValueX && qreal(tofs[i]) > 0)
			maxValueX = x_p+bw;
		if(qreal(tofs[i])+10>maxValueY)
			maxValueY = qreal(tofs[i])+10;

		line->append(x, qreal(tofs[i]));
		line->append(x_p, qreal(tofs[i]));
	}
	line->append(qreal(maxBin*bw+bw),0);


	xAxis->setRange(floor(minValueX/bw)*bw-bw, ceil(maxValueX/bw)*bw+bw);
	yAxis->setRange(0, uint(maxValueY*8/7) - uint(maxValueY*8/7)%yStep +yStep);

	binned_changed = false;
}

//When tagger device is started
void TofHistogram::newTagger()
{
	tag_pos = 0;
	tofs.fill(0);
	if(!taggerUpdateTimer->isActive())
		taggerUpdateTimer->start(taggerUpdateTime);
}

void TofHistogram::closeEvent(QCloseEvent *event)
{
	emit closing(true);
}