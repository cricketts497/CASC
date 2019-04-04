#include "include/fakeTagger.h"

FakeTagger::FakeTagger(int rate, QString file_path, QMutex * file_mutex, CascConfig * config, QObject *parent) :
LocalDataDevice(file_path, file_mutex, QString("faketagger"), config, parent),
rate(rate),
file_path(file_path),
timer(new QTimer(this)),
hits_per_packet(2),
timestamp(0),
packet_hits(0),
flag(0),
hit_data(0b00000000001001110001000000000000),
hit_data2(0b00000000110000110101000000000000),
// packets_to_read(3),
file_mutex(file_mutex),
packets(0)
{
	int interval = 999/rate;
	
	time = new QDateTime();
	
	connect(timer, SIGNAL(timeout()), this, SLOT(hit()));
	
	// //check input
	// char chan = 65 + (hit & 0xf);
	// int flag = hit>>4 & 0xf;
	// int ts = hit>>8 & 0xffffff;
	// std::cout << chan << flag << ts << std::endl;
	
	QMutexLocker file_locker(file_mutex);
	fake_tag_temp_file = new QFile(file_path);
	if(!fake_tag_temp_file->open(QIODevice::WriteOnly)){
		storeMessage(QString("FAKE TAGGER ERROR: init: file->open(write)"), true);
		return;
	}
	QDataStream out(fake_tag_temp_file);
	qint64 header = time->currentMSecsSinceEpoch();//*2e6;
	out << header;
	fake_tag_temp_file->close();
	
	timer->start(interval);
}

void FakeTagger::hit()
{
	//hit_data timestamp: 10000*500ps=5us, flag: 0, channel: A
	//          timestamp           flag chan
	//0000 0000 0010 0111 0001 0000 0000 0000
	//hit_data2 timestamp: 50000*500ps=25us, flag: 0, channel: A
	//          timestamp           flag chan
	//0000 0000 1100 0011 0101 0000 0000 0000
	//append to end of packet data
	
	packet_hits++;
	
	if(packet_hits >= hits_per_packet){
		newPacket();
	}
}

//need to make this thread safe if adding threading to rest
void FakeTagger::newPacket()
{
	// increase the coarse packet timestamp
	// interval in ms, timestamp in multiples of 500ps
	// timestamp += timestamp_interval*packet_hits;
	timestamp = time->currentMSecsSinceEpoch();//*2e6;

	QMutexLocker file_locker(file_mutex);
	if(!fake_tag_temp_file->open(QIODevice::Append)){
		emit device_message(QString("FAKE TAGGER ERROR: newPacket: file->open(append)"));
		emit device_fail();
		return;
	}
	QDataStream out(fake_tag_temp_file);
	
	// packet header
	out << timestamp;
	out << packet_hits;
	out << flag;
	
	for(uint i=0; i<packet_hits/2; i++){
		out << hit_data;
		out << hit_data2;
	}
	
	fake_tag_temp_file->close();

	packet_hits = 0;
	packets++;

}


