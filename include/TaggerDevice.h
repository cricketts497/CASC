#ifndef TAGGER_DEVICE
#define TAGGER_DEVICE

#include <QtWidgets>
#include <QFile>
// #include "include/timetagger4_interface.h"

class TaggerDevice : public QTimer
{
	Q_OBJECT
public:
	TaggerDevice(const int read_interval, const QString file_path, QMainWindow * parent = nullptr);
	~TaggerDevice();
	void emitTaggerError();
	bool start_card();
	void stop_card();

signals:
	void tagger_message(QString error);
	void tagger_fail();
	
private slots:
	void readPackets();

private:
	int initCard();
	int sendBoardInfo();
	int startCapture();
	
	int stopCapture();
	int closeCard();
	
	QFile * tag_temp_file;

	// timetagger4_device * device;
	// int error_code;
	// const char * err_message;
	
	// timetagger4_read_in read_config;
	// timetagger4_read_out read_data;
	
	bool card_running;
	bool capture_running;

};

#endif // TAGGER_DEVICE