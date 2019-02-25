#ifndef FAKE_TAGGER
#define FAKE_TAGGER

#include <QTimer>

class QMainWindow;
class QFile;
class QMutex;

class FakeTagger: public QTimer
{
	Q_OBJECT
	
public:
	FakeTagger(int rate, const QString file_path, QMutex * file_mutex, QMainWindow *parent=nullptr);
	
private slots:
	void hit();

signals:
	// void updateHits(int packetHits);
	// void update(bool newPackets);
	void tagger_fail();
	
private:
	void newPacket();

	const uint hits_per_packet;

	//raw timestamp, increases with timer timeout
	QDateTime *time;
	qint64 timestamp;
	uint timestamp_interval;

	quint64 packet_hits;
	
	//flag for the packet, set zero
	uchar flag;

	//the hit data
	const quint32 hit_data;
	const quint32 hit_data2;

	QMutex * file_mutex;
	QFile *fake_tag_temp_file;

	// const uint packets_to_read;
	uint packets;	
};
#endif //FAKE_TAGGER