#ifndef FAKE_TAGGER
#define FAKE_TAGGER

#include <QTimer>

class QMainWindow;
class QFile;

class FakeTagger: public QTimer
{
	Q_OBJECT
	
public:
  FakeTagger(int rate, QMainWindow *parent);
	
private slots:
	void hit();

signals:
	void updateHits(int packetHits);
	
private:
	void newPacket();

	const uint hits_per_packet;

	//raw timestamp, increases with timer timeout
	quint64 timestamp;
	uint timestamp_interval;
	
	//flag for the packet, set zero
	uchar flag;

	//the hit data
	const quint32 hit_data;

	quint64 packet_hits;
	
	QFile *fake_tag_temp_file;	
};
#endif //FAKE_TAGGER