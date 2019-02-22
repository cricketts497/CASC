#ifndef TAGGER_DEVICE
#define TAGGER_DEVICE

#include <QObject>
#include <QTextStream>
#include "include/timetagger4_interface.h"

class TaggerDevice : public QObject
{
	Q_OBJECT
public:
	TaggerDevice(QObject * parent = nullptr);
	~TaggerDevice();
	void emitTaggerError();
	bool start();
	void stop();

signals:
	void tagger_message(QString error);

private:
	int initCard();
	int sendBoardInfo();
	int closeCard();

	timetagger4_device * device;
	int error_code;
	const char * err_message;

	bool card_running;

};

#endif // TAGGER_DEVICE