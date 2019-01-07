#include "include/fakeTagger.h"
#include <QtWidgets>
#include <QFile>

FakeTagger::FakeTagger(int rate, const QString file_path, QMainWindow *parent) :
QTimer(parent),
hits_per_packet(10),
timestamp(0),
flag(0),
hit_data(0b00000000001001110001000000000000),
packet_hits(0),
packets_to_read(30),
packets(0)
{
	uint interval = 1000/rate;
	timestamp_interval = interval*2000000;
	
	connect(this, SIGNAL(timeout()), this, SLOT(hit()));
	
	// //check input
	// char chan = 65 + (hit & 0xf);
	// int flag = hit>>4 & 0xf;
	// int ts = hit>>8 & 0xffffff;
	// std::cout << chan << flag << ts << std::endl;
	
	fake_tag_temp_file = new QFile(file_path);
	if(!fake_tag_temp_file->open(QIODevice::WriteOnly)){
		qDebug() << "Couldn't open new tagger file";
		return;
	}
	QDataStream out(fake_tag_temp_file);
	out << QString("TAGGER");
	fake_tag_temp_file->close();
	
	start(interval);
}

void FakeTagger::hit()
{
	//hit timestamp: 10000*500ps=5us, flag: 0, channel: A
	//          timestamp           flag chan
	//0000 0000 0010 0111 0001 0000 0000 0000
	//append to end of packet data
	
	packet_hits++;

	emit updateHits(packet_hits);
	
	if(packet_hits >= hits_per_packet){
		newPacket();
	}

	if(packets >= packets_to_read){
		emit update(true);
		packets = 0;
	}
}

//need to make this thread safe if adding threading to rest
void FakeTagger::newPacket()
{
	if(!fake_tag_temp_file->open(QIODevice::Append)){
		qDebug() << "Couldn't open file to append data";
		return;
	}
	QDataStream out(fake_tag_temp_file);
	
	//packet header
	// out.writeRawData(&timestamp,8);
	// out.writeRawData(&packet_hits, 8);
	// out.writeRawData(&flag, 1);
	
	// for(int i=0; i<packet_hits; i++){
		// out.writeRawData(&hit, 4);
	// }
	
	//packet header
	out << timestamp;
	out << packet_hits;
	out << flag;
	
	for(uint i=0; i<packet_hits; i++){
		out << hit_data;
	}
	
	fake_tag_temp_file->close();

	// emit update(true);

	//increase the coarse packet timestamp
	//interval in ms, timestamp in multiples of 500ps
	timestamp += timestamp_interval*packet_hits;
	packet_hits = 0;
	packets++;
}

