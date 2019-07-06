#ifndef SIMPLE_GRAPH_H
#define SIMPLE_GRAPH_H

#include "include/ZoomChartView.h"
#include <QtCharts>

class SimpleGraph : public QWidget
{
    Q_OBJECT
    
public:
    SimpleGraph(QWidget * parent = nullptr);
    
private:
    QStringList filePaths;
    QList<&QMutex> fileMutexes;
    
    bool zoomed;
    int xStep;
    int yStep;
    int binWidth;
    const int maxBinWidth;
    qint64 filePos;
    qreal start_time;
    int totalPoints;
    
    int minValueX;
    int minValueY;
    int maxValueX;
    int maxValueY;
    
    ZoomChartView * chartView;
	QScatterSeries * series;
    QValueAxis * xAxis;
	QValueAxis *yAxis;
  
    QVector<qreal> sumTimes;
    QVector<qreal> sumValues;
    QVector<qreal> sumCounts;
};


#endif // SIMPLE_GRAPH_H