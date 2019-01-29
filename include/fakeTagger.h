#ifndef FAKE_TAGGER
#define FAKE_TAGGER

#include <QTimer>

class QMainWindow;
class QFile;

class FakeTagger: public QTimer
{
	Q_OBJECT
	
public:
	FakeTagger(int rate, const QString file_path, QMainWindow *parent);
	
private slots:
	void hit();

signals:
	void updateHits(int packetHits);
	// void update(bool newPackets);
	
private:
	void newPacket();

	const uint hits_per_packet;

	//raw timestamp, increases with timer timeout
	QDateTime *time;
	quint64 timestamp;
	uint timestamp_interval;

	quint64 packet_hits;
	
	//flag for the packet, set zero
	uchar flag;

	//the hit data
	const quint32 hit_data;

	QFile *fake_tag_temp_file;

	// const uint packets_to_read;
	uint packets;	
};
#endif //FAKE_TAGGER