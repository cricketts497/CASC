#include "include/TaggerDevice.h"

TaggerDevice::TaggerDevice(QObject * parent) :
QObject(parent),
card_running(false)
{

}

TaggerDevice::~TaggerDevice()
{
	if(card_running)
		stop();
}

bool TaggerDevice::start()
{
	if(initCard())
		return false;
	if(sendBoardInfo())
		return false;
	return true;
}

void TaggerDevice::stop()
{
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
		return 1;
	}

	params.buffer_size[0] = 8 * 1024 * 1024;//8MByte packet buffer size
	params.board_id = 0;//first board
	params.card_index = 0;//find first board in system

	//initialise the card
	device = timetagger4_init(&params, &error_code, &err_message);
	if(error_code != CRONO_OK){
		emit tagger_message(QString("TAGGER ERROR: timetagger4_init: %1").arg(err_message));
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

	QString message_str;
	QTextStream message(&message_str);
	message << "Tagger: Board configuration: TimeTagger4-";
	switch(staticinfo.board_configuration)
	{
		// case TIMETAGGER4_1G_BOARDCONF:
		case 0:
			message << "1G";
			break;
		// case TIMETAGGER4_2G_BOARDCONF:
		case 1:
			message << "2G";
			break;
		default:
			message << "unknown";
	}
	emit tagger_message(message.readAll());

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

int TaggerDevice::closeCard()
{
	timetagger4_close(device);
	card_running = false;
	return 0;
}