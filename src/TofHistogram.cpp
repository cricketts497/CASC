#include "include/TofHistogram.h"
#include <QtWidgets>


TofHistogram::TofHistogram(const QString tag_path, QWidget * parent) :
QDockWidget("TOF Histogram", parent),
yStep(4),
maxValueY(10.0),
zoomed(false),
tag_pos(0),
taggerUpdateTime(20),
binWidth(1),//us
maxBin(0),
binned_changed(false),
graphUpdateTime(45)
{
	// setFloating(true);
	// setMinimumSize(300,300);
	setFixedSize(500,500);

	QWidget * widget = new QWidget(this);
	setWidget(widget);

	QVBoxLayout * layout = new QVBoxLayout;
	widget->setLayout(layout);

	TofChartView * chartView = new TofChartView(this);

	line = new QLineSeries(this);
	line->setPen(QPen(Qt::black));
	chartView->chart()->addSeries(line);

	window_line = new QLineSeries(this);
	window = new QAreaSeries(this);
	window->setBorderColor(Qt::blue);
	window->setColor(QColor(0,0,255,70));//rgba colour, alpha=70/255
	window->setUpperSeries(window_line);
	chartView->chart()->addSeries(window);
	connect(window, SIGNAL(doubleClicked(const QPointF&)), this, SLOT(removeSelectionWindow()));

	chartView->chart()->legend()->setVisible(false);
	connect(chartView, SIGNAL(new_zoom(bool)), this, SLOT(chartZoomed()));
	connect(chartView, SIGNAL(selectionWindow(qreal,qreal)), this, SLOT(newSelectionWindow(qreal,qreal)));
	layout->addWidget(chartView);

	xAxis = new QValueAxis(this);
	xAxis->setTitleText("Time of flight / us");
	xAxis->setRange(-5,70);
	xAxis->setLabelFormat("%.3f");
	// chartView->chart()->setAxisX(xAxis, line);
	chartView->chart()->addAxis(xAxis, Qt::AlignBottom);
	line->attachAxis(xAxis);
	window->attachAxis(xAxis);

	yAxis = new QValueAxis(this);
	yAxis->setTitleText("Counts");
	yAxis->setRange(0,300);
	// yAxis->setRange(0,150);
	yAxis->setLabelFormat("%.3f");
	// yAxis->setTickCount(16);
	// chartView->chart()->setAxisY(yAxis, line);
	chartView->chart()->addAxis(yAxis, Qt::AlignLeft);
	line->attachAxis(yAxis);
	window->attachAxis(yAxis);

	QHBoxLayout * bottomLayout = new QHBoxLayout;

	QPushButton *resetButton = new QPushButton("Reset axes", this);
	connect(resetButton, SIGNAL(clicked()), this, SLOT(resetAxes()));

	QLabel * binWidthLabel = new QLabel("Bin width / us:", this);
	binWidthEdit = new QSpinBox(this);
	binWidthEdit->setValue(binWidth);
	binWidthEdit->setRange(1,500);
	connect(binWidthEdit, SIGNAL(editingFinished()), this, SLOT(changeBinWidth()));

	bottomLayout->addWidget(resetButton);
	bottomLayout->addStretch();
	bottomLayout->addWidget(binWidthLabel);
	bottomLayout->addWidget(binWidthEdit);
	layout->addLayout(bottomLayout);

	tofs = QVector<int>(nBins, 0);

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

	int bw = binWidth*2000;

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

			for(int i=bw; i<nBins*bw; i+=bw){
				if(offset<i){
					int bin = i/bw;

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
	maxValueY = 10;
	qreal minValueX = 0;
	qreal maxValueX = 50;

	for(int i=0; i<=maxBin; i++){
		if(tofs[i] < 0)
			continue;

		//convert from units of 10ns to us
		qreal x = qreal(i*binWidth);
		qreal x_p = qreal(i*binWidth+binWidth);
		
		if(x < minValueX && qreal(tofs[i]) > 0)
			minValueX = x-binWidth;
		if(x_p+binWidth>maxValueX && qreal(tofs[i]) > 0)
			maxValueX = x_p+binWidth;
		if(qreal(tofs[i])+10>maxValueY)
			maxValueY = qreal(tofs[i])+10;

		line->append(x, qreal(tofs[i]));
		line->append(x_p, qreal(tofs[i]));
	}
	line->append(qreal(maxBin*binWidth+binWidth),0);

	if(!zoomed){
		xAxis->setRange(floor(minValueX/binWidth)*binWidth-binWidth, ceil(maxValueX/binWidth)*binWidth+binWidth);
		yAxis->setRange(0, uint(maxValueY*8/7) - uint(maxValueY*8/7)%yStep +yStep);
	}

	binned_changed = false;
}

void TofHistogram::changeBinWidth()
{
	binWidth = binWidthEdit->value();

	//clear the binned data
	tofs = QVector<int>(nBins, 0);

	//re-read all the data
	tag_pos = 0;
	updateTag();

	resetAxes();
}

void TofHistogram::chartZoomed()
{
	zoomed = true;
}

void TofHistogram::resetAxes()
{
	xAxis->setRange(-5,70);
	yAxis->setRange(0,300);	

	zoomed = false;
	binned_changed = true;
	updateHist();
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

void TofHistogram::newSelectionWindow(qreal left, qreal right)
{
	//update the selection window lines
	QList<QPointF> window_list = {QPointF(left, yAxis->max()*10000), QPointF(right, yAxis->max()*10000)};
	window_line->replace(window_list);

	emit selectionWindow(left, right);
}

void TofHistogram::removeSelectionWindow()
{
	//reset to default values
	qreal left = 0.0;
	qreal right = 50.0;

	window_line->clear();

	emit selectionWindow(left, right);
}