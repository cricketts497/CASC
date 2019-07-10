#ifndef SIMPLE_GRAPH_H
#define SIMPLE_GRAPH_H

#include "include/ZoomChartView.h"
#include <QtCharts>

class SimpleGraph : public QWidget
{
    Q_OBJECT
    
public:
    SimpleGraph(QStringList filePaths, QList<QMutex*> fileMutexes, QWidget * parent = nullptr);
    
private slots:
    void chartZoomed();
    void changeBinWidth();
    void resetAxes();
    void changeYAxis(int newIndex);
    void updateGraph();
    
    
private:
    QStringList filePaths;
    QList<QMutex*> fileMutexes;
    
    bool zoomed;
    int xStep;
    int yStep;
    int binWidth;
    const int maxBinWidth;
    qint64 filePos;
    qreal start_time;
    int nPoints;
    
    int graphUpdateTime;
    int yAxisIndex;
    int binEdge;
    
    int minValueX;
    int minValueY;
    int maxValueX;
    int maxValueY;
    
    ZoomChartView * chartView;
	QScatterSeries * series;
    QValueAxis * xAxis;
	QValueAxis *yAxis;
    QSpinBox * binWidthEdit;
  
    QVector<qreal> sumTimes;
    QVector<qreal> sumValues;
    QVector<qreal> sumCounts;
};


#endif // SIMPLE_GRAPH_H