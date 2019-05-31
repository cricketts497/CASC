#ifndef GENERIC_GRAPH
#define GENERIC_GRAPH

#include "include/ZoomChartView.h"
#include <QtCharts>

class GenericGraph : public QWidget
{
	Q_OBJECT

public:
	GenericGraph(const QString tag_path, const QString pdl_path, const QString heinzinger30k_path, const QString heinzinger20k_path, QMutex * tag_mutex, QMutex * pdl_mutex, QMutex * heinzinger30k_mutex, QMutex * heinzinger20k_mutex, QMainWindow *parent);
	void newTagger();
	void newPdl();
	// void closedPdl();
    void newHeinzinger30k();
    void newHeinzinger20k();

signals:
	void newEdge(qreal edge);
	void graph_message(QString message);

private slots:
	void updateTag();
	void updatePdl();
    void updateHeinzinger30k();
    void updateHeinzinger20k();
    
	void updateGraph();
	void changeBinWidth();
	void changeXAxis(int newIndex);
	void changeYAxis(int newIndex);
	void chartZoomed();
	void resetAxes();
	void newSelectionWindow(qreal left, qreal right);
	void emitGraphMessage(QString message);

private:
	//widget layout
	QGridLayout *layout;

	//graph data
	ZoomChartView *chartView;
	QScatterSeries *series;
	QBoxPlotSeries *errorSeries;
	QValueAxis *xAxis;
	QValueAxis *yAxis;
	QValueAxis *rateAxis;

	//binWidth editing
	QLabel *binWidthLabel;
	QSpinBox *binWidthEdit;
	const uint maxBinWidth;

	//axes editing
	QComboBox *yAxisCombo;
	uint yAxisIndex;
	QComboBox *xAxisCombo;
	uint xAxisIndex;

	//graph updating
	const uint graphUpdateTime;
	uint xStep;
	uint yStep;
	bool binned_changed;
	bool zoomed;

	//binned data
	int binWidth;
	QVector<QVector<qreal>> binEdges;
	int bindex;
	int time_index;
	qreal start_time;
	void appendZeros();
	void prependZeros();
	void clearAll();

	void checkMinMax(qreal x, qreal y);
	qreal maxValueX;
	qreal maxValueY;
	qreal minValueX;
	qreal minValueY;
	
	//tagger data
	void binTagger_byTime(qreal time, int packet_hits);
	void binTagger_byPdl(qreal time, int packet_hits);
	QFile *tag_file;
	QMutex *tag_mutex;
	qint64 tag_pos;
	bool tagger_started;
	const uint taggerUpdateTime;
	QTimer *taggerUpdateTimer;
	qreal lastPacketTime;

	QVector<qreal> tag_times;
	QVector<uint> counts;
	QVector<qreal> delts;

	//tagger tof gating, us
	qreal max_tof;
	qreal min_tof;

	//pdl data
	// void binPdl_byTime(qreal time, quint64 pdl_wavenumber);
	void binPdl_byPdl(qreal time, quint64 pdl_wavenumber);
	QFile *pdl_file;
	QMutex * pdl_mutex;
	qint64 pdl_pos;
	bool pdl_started;
	const uint pdlUpdateTime;
	QTimer *pdlUpdateTimer;

	QVector<uint> pdl_wavenumbers;
	QVector<uint> pdl_counts;
	
	QVector<uint> binEdges_pdl;
	
	// uint pdl_wavenumber_edge;

    //heinzinger 30k data
    void binHeinzinger30k_byTime(qreal time, qreal voltage);
    QFile * heinzinger30k_file;
    QMutex * heinzinger30k_mutex;
    qint64 heinzinger30k_pos;
    bool heinzinger30k_started;
    QTimer * heinzinger30k_updateTimer;
    
    QVector<qreal> heinzinger30k_times;
    QVector<qreal> heinzinger30k_voltages;
	QVector<qreal> heinzinger30k_counts;
    
    //heinzinger 20k data
    void binHeinzinger20k_byTime(qreal time, qreal voltage);
    QFile * heinzinger20k_file;
    QMutex * heinzinger20k_mutex;
    qint64 heinzinger20k_pos;
    bool heinzinger20k_started;
    QTimer * heinzinger20k_updateTimer;
    
    QVector<qreal> heinzinger20k_times;
    QVector<qreal> heinzinger20k_voltages;
	QVector<qreal> heinzinger20k_counts;

    const uint heinzinger_updateTime;

	//debug
	// int holder = 1e4;
};

#endif //GENERIC_GRAPH