#include "include/SimpleGraph.h"

//indices: heinzinger30k, heinzinger20k, BLTest

SimpleGraph::SimpleGraph(QStringList filePaths, QList<QMutex*> fileMutexes, QWidget * parent) :
QWidget(parent),
filePaths(filePaths),
fileMutexes(fileMutexes),
zoomed(false),
xStep(4),
yStep(4),
binWidth(2),
maxBinWidth(10000),
filePos(0),
start_time(0),
nPoints(0),
maxValueX(-1e8),
maxValueY(-1e8),
minValueX(1e8),
minValueY(1e8),
graphUpdateTime(1000)
{
	QVBoxLayout * layout = new QVBoxLayout(this);

	//main chart
	chartView = new ZoomChartView(this);
	chartView->chart()->legend()->setVisible(false);
	connect(chartView, SIGNAL(new_zoom(bool)), this, SLOT(chartZoomed()));
	layout->addWidget(chartView);

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
	// yAxis->setTitleText("Counts");
	yAxis->setRange(0,yStep);
	yAxis->setLabelFormat("%.3f");
	chartView->chart()->setAxisY(yAxis, series);

	//bin width edit box, a combo box to change the y-axis and a reset button for zoom
	QHBoxLayout * bottomLayout = new QHBoxLayout;

	QLabel * binWidthLabel = new QLabel("Bin width / s:", this);
	binWidthEdit = new QSpinBox(this);
	binWidthEdit->setValue(binWidth);
	binWidthEdit->setRange(1,maxBinWidth);
	connect(binWidthEdit, SIGNAL(editingFinished()), this, SLOT(changeBinWidth()));

	QPushButton * resetButton = new QPushButton("Reset axes", this);
	connect(resetButton, SIGNAL(clicked()), this, SLOT(resetAxes()));

	QLabel * yAxisLabel = new QLabel("y:", this);
	QComboBox * yAxisCombo = new QComboBox(this);
    yAxisCombo->addItem("Heinzinger 30k voltage / V");
	yAxisCombo->addItem("Heinzinger 20k voltage / V");
    yAxisCombo->addItem("BLTest pump temperature / C");
	connect(yAxisCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeYAxis(int)));

	bottomLayout->addWidget(resetButton);
	bottomLayout->addWidget(yAxisLabel);
	bottomLayout->addWidget(yAxisCombo);
	bottomLayout->addStretch();
	bottomLayout->addWidget(binWidthLabel);
	bottomLayout->addWidget(binWidthEdit);
	layout->addLayout(bottomLayout);
    
	QTimer * graphUpdateTimer = new QTimer(this);
	connect(graphUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGraph()));
	graphUpdateTimer->start(graphUpdateTime);
}

void SimpleGraph::chartZoomed()
{
	zoomed = true;
}

//When the parameter on the y-axis is changed using the combo box
void SimpleGraph::changeYAxis(int newIndex)
{
    if(newIndex == 0){
        yAxis->setTitleText("Heinzinger 30k voltage / V");
    }else if(newIndex == 1){
        yAxis->setTitleText("Heinzinger 20k voltage / V");
    }else if(newIndex == 2){
        yAxis->setTitleText("BLTest pump temperature / C");
    }
    changeBinWidth();
	yAxisIndex = newIndex;
}

void SimpleGraph::changeBinWidth()
{
	binWidth = binWidthEdit->value();
    
	//clear the current binned data
	sumTimes.clear();
    sumValues.clear();
    sumCounts.clear();
	start_time = 0;
    
    filePos = 0;
    
	resetAxes();
}

//when the reset axes push button is pressed
void SimpleGraph::resetAxes()
{
	xAxis->setRange(0,xStep);
	yAxis->setRange(0,yStep);

	zoomed = false;
	updateGraph();
}

void SimpleGraph::updateGraph()
{
    qint64 current_pos = filePos;
    
    QFile dataFile(filePaths.at(yAxisIndex), this);
    QMutex * fileMutex = fileMutexes.at(yAxisIndex);
    
    bool locked = fileMutex->tryLock();
    if(!locked)
        return;
    
    if(!dataFile->open(QIODevice::ReadOnly)){
		fileMutex->unlock();
		return;        
    }
    
    dataFile->seek(filePos);
    QDataStream in(dataFile);
    
    //heinzinger file
    if(yAxisIndex == 0 || yAxisIndex == 1){
        if(filePos == 0){
            qint64 header;
            in >> header;
            if(start_time < 1)
                start_time = qreal(header)/1000;
        }
        qint64 timestamp;
        quint64 voltage_applied;
        quint64 voltage_decimal_applied;
        while(!dataFile->atEnd()){
            in >> timestamp >> voltage_applied >> voltage_decimal_applied;

            //timestamp since epoch in ms
            qreal time = qreal(timestamp)/1000;
            time -= start_time;
            
            qreal voltage = QString("%1.%2").arg(voltage_applied).arg(voltage_decimal_applied).toDouble();
            
            if(time > binEdge){
                sumTimes.append(time);
                sumValues.append(voltage);
                sumCounts.append(1);
                binEdge += binWidth;
            }else{
                sumTimes.last() += time;
                sumValues.last() += voltage;
                sumCounts.last()++;
            }
        }
    }else{
        return;
    }

    filePos = dataFile->pos();
    dataFile->close();
    fileMutex->unlock();
    
    //new data
    if(filePos > current_pos){
        int start;
        if(nPoints>0){
            nPoints--;
            series->remove(nPoints);
        }
        for(int i=nPoints; i<sumTimes.size(); i++){
            if(sumCounts.at(i) == 0)
                continue;
            
            qreal x = sumTimes.at(i)/sumCounts.at(i);
            qreal y = sumValues.at(i)/sumCounts.at(i);
            
            if(x > maxValueX)
                maxValueX = x;
            if(y > maxValueY)
                maxValueY = y;
            if(x < minValueX)
                minValueX = x;
            if(y < minValueY)
                minValueY = y;
            
            series->append(x,y);
            
            nPoints++;
        }
    }
    
    if(!zoomed){
		// xAxis->setMax(uint(maxValueX)- uint(maxValueX)%xStep +2*xStep);
		// xAxis->setRange(uint(minValueX)-uint(minValueX)%xStep-xStep, uint(maxValueX)- uint(maxValueX)%xStep +xStep);
		// xAxis->setRange(uint(minValueX)-uint(minValueX)%xStep-xStep, uint(maxValueX)- uint(maxValueX)%xStep +xStep);
		// xAxis->setMax(xAxis->max()+xStep);
		// axisX->setTickCount(axisX->max()+1/5+1);
		// xAxis->setRange(floor(minValueX/binWidth)*binWidth-binWidth, ceil(maxValueX/binWidth)*binWidth+binWidth);
		xAxis->setRange(0, ceil(maxValueX/binWidth)*binWidth+binWidth);

		// yAxis->setMax(uint(maxValueY*8/7) - uint(maxValueY*8/7)%yStep +2*yStep);
		// yAxis->setRange(uint(minValueY)-uint(minValueY)%yStep, uint(maxValueY) - uint(maxValueY)%yStep +yStep);
		// yAxis->setRange(0, uint(maxValueY*8/7) - uint(maxValueY*8/7)%yStep +yStep);
		// yAxis->setRange(0, maxValueY*8/7);
		// yAxis->setMax(yAxis->max()+yStep);
		// axisY->setTickCount(axisX->max()+1/5+1);
        yAxis->setRange(floor(minValueY/binWidth)*binWidth-binWidth, ceil(maxValueY/binWidth)*binWidth+binWidth);
	}
}

