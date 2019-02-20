#ifndef TAGGER_DEVICE
#define TAGGER_DEVICE

#include <QObject>
// #include "include/timetagger4_interface.h"

class TaggerDevice : public QObject
{
	Q_OBJECT
public:
	TaggerDevice(QObject * parent = nullptr);
	~TaggerDevice();
	void emitTaggerError();
	void start();
	void stop();

signals:
	void tagger_message(QString error);

private:
	void initCard();
	void sendBoardInfo();
	void closeCard();

	// timetagger4_device * device

	bool card_running;

};

#endif // TAGGER_DEVICE