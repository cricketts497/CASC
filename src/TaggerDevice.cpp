#include "include/TaggerDevice.h"

TaggerDevice::TaggerDevice(const int read_interval, const QString file_path, QMainWindow *parent) :
QTimer(parent),
// packet_count(0),
// empty_packets(0),
// packets_with_errors(0),
// last_read_no_data(false),
card_running(false),
capture_running(false)
{
	QDateTime * start_time = new QDateTime();
	
	//file to write the tagger data to
	tag_temp_file = new QFile(file_path);
	if(!tag_temp_file->open(QIODevice::WriteOnly)){
		emit tagger_message(QString("TAGGER ERROR: Couldn't open tagger file to write header"));
		return;
	}
	QDataStream out(tag_temp_file);
	qint64 header = start_time->currentMSecsSinceEpoch();
	out << header;
	tag_temp_file->close();
	
	//timer to set interval between card reads
	connect(this, SIGNAL(timeout()), this, SLOT(readPackets()));
	setInterval(read_interval);
}

TaggerDevice::~TaggerDevice()
{
	stop_card();
}

bool TaggerDevice::start_card()
{
	if(initCard())
		return false;
	
	if(sendBoardInfo()){
		closeCard();
		return false;
	}
	
	if(startCapture()){
		closeCard();
		return false;
	}
	
	return true;
}

void TaggerDevice::stop_card()
{
	stopCapture();
	closeCard();
}

void TaggerDevice::emitTaggerError()
{
	emit tagger_message(QString("TAGGER ERROR: This is some error message you may get from the tagger card: %1").arg("some string"));
}

/////////////////////////////////////////////////////////////////////////////////

int TaggerDevice::initCard()
{
	int status;
	timetagger4_init_parameters params;
	status = timetagger4_get_default_init_parameters(&params);
	if(status != CRONO_OK){
		emit tagger_message(QString("TAGGER ERROR: timetagger4_get_default_init_parameters"));
		emit tagger_fail();
		return 1;
	}

	params.buffer_size[0] = 8 * 1024 * 1024;//8MByte packet buffer size
	params.board_id = 0;//first board
	params.card_index = 0;//find first board in system

	//initialise the card
	device = timetagger4_init(&params, &error_code, &err_message);
	if(error_code != CRONO_OK){
		emit tagger_message(QString("TAGGER ERROR: timetagger4_init: %1").arg(err_message));
		emit tagger_fail();
		return 1;
	}
	card_running = true;
	
	//config
	timetagger4_configuration config;
	timetagger4_get_default_configuration(device, &config);

	//set the config for the channels, start and stop
	int channels=2;
	for(int i=0; i<channels; i++){
		config.channel[i].enabled = true;
		config.channel[i].start = 0;
		config.channel[i].stop = 30000;//recording window stops after 15us
		
		//measure only falling edge
		config.trigger[i+1].falling = 1;
		config.trigger[i+1].rising = 0;
	}
	//start channel
	config.trigger[0].falling = 1;
	config.trigger[0].rising = 0;

	//write this config to the board
	status = timetagger4_configure(device, &config);
	if(status != CRONO_OK){
		emit tagger_message(QString("TAGGER ERROR: timetagger4_configure: %1").arg(err_message));
		emit tagger_fail();
		return error_code;
	}
	
	return 0;
}

int TaggerDevice::sendBoardInfo()
{
	//send some board info
	timetagger4_static_info staticinfo;
	int status = timetagger4_get_static_info(device, &staticinfo);
	if(status != CRONO_OK){
		emit tagger_message(QString("TAGGER ERROR: timetagger4_get_static_info: %1").arg(err_message));
		return error_code;
	}
	emit tagger_message(QString("Tagger: Board Serial: %1.%2").arg(staticinfo.board_serial>>24).arg(staticinfo.board_serial&0xffffff));

	// QString message_str;
	// QTextStream message(&message_str);
	// message << "Tagger: Board configuration: TimeTagger4-";
	// switch(staticinfo.board_configuration)
	// {
		// case TIMETAGGER4_1G_BOARDCONF:
			// message << "1G";
			// break;
		// case TIMETAGGER4_2G_BOARDCONF:
			// message << "2G";
			// break;
		// default:
			// message << "unknown";
	// }
	// emit tagger_message(message.readAll());

	emit tagger_message(QString("Tagger: Board Revision: %1").arg(staticinfo.board_revision));
	emit tagger_message(QString("Tagger: Firmware Revision: %1.%2").arg(staticinfo.firmware_revision).arg(staticinfo.subversion_revision));
	emit tagger_message(QString("Tagger: Driver Revision: %1.%2.%3.%4").arg(((staticinfo.driver_revision>>24)&255)).arg(((staticinfo.driver_revision>>16)&255)).arg(((staticinfo.driver_revision>>8)&255)).arg((staticinfo.driver_revision&255)));

	timetagger4_param_info parinfo;
	status = timetagger4_get_param_info(device, &parinfo);
	if(status != CRONO_OK){
		emit tagger_message(QString("TAGGER ERROR: timetagger4_get_param_info: %1").arg(err_message));
		return error_code;
	}
	emit tagger_message(QString("Tagger: TDC binsize: %1 ps").arg(parinfo.binsize));

	return 0;
}

int TaggerDevice::startCapture()
{
	//auto acknowledge data as processed: on read old packet pointers are no longer valid
	read_config.acknowledge_last_read = 1;
	
	int status = timetagger4_start_capture(device);
	if(status != CRONO_OK){
		emit tagger_message(QString("TAGGER ERROR: timetagger4_start_capture: %1").arg(timetagger4_get_last_error_message(device)));
		emit tagger_fail();
		return status;
	}
	capture_running = true;
	
	//start the packet reading timer
	start();
	
	return 0;
}

void TaggerDevice::readPackets()
{
	int status = timetagger4_read(device, &read_config, &read_data);
	if(status == CRONO_READ_NO_DATA){
		emit tagger_message(QString("Tagger: readPackets: No data"));
		return;
	}else if(status == CRONO_READ_TIMEOUT){
		emit tagger_message(QString("Tagger: readPackets: Timeout"));
		return;
	}else if(status != CRONO_READ_OK){
		emit tagger_message(QString("TAGGER ERROR: readPackets: %2").arg(read_data.error_message));
		emit tagger_fail();
		return;
	}
	
	quint64 timestamp;
	uchar flags;
	int packet_hits;
	
	if(!tag_temp_file->open(QIODevice::Append)){
		emit tagger_message(QString("TAGGER ERROR: Couldn't open tagger file to append data"));
		emit tagger_fail();
		return;
	}
	QDataStream out(tag_temp_file);
	
	//iterate over all the packets in the read and write them to a temporary file
	crono_packet * p = read_data.first_packet;
	while(p <= read_data.last_packet){
		if(p->channel != uchar(0) || p->type != uchar(6)){
			emit tagger_message(QString("Tagger: Bad Packet"));
			p = crono_next_packet(p);
			continue;
		}
		
		timestamp = p->timestamp;
		out << timestamp;
		
		packet_hits = 2 * (p->length);
		//check for odd number of hits in the packet, given by flags=1
		if((flags & 0x1) == 1)
			packet_hits -= 1;
		out << packet_hits;
		
		flags = p->flags;
		out << flags;
		
		quint32 * packet_data = (quint32*)(p->data);
		for(int i=0; i<packet_hits; i++){
			quint32 * hit = (packet_data + i);
			out << *hit;
		}
		
		//go to the next packet in the read
		p = crono_next_packet(p);
	}
	
	tag_temp_file->close();
}

int TaggerDevice::stopCapture()
{
	//stop the packet reading timer
	if(isActive())
		stop();
	
	if(capture_running){
		timetagger4_stop_capture(device);
		capture_running = false;
	}
	
	return 0;
}

int TaggerDevice::closeCard()
{
	if(card_running){
		timetagger4_close(device);
		card_running = false;
	}
	return 0;
}











