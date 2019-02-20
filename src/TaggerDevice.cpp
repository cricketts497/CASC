#include "include/TaggerDevice.h"
// #include <QString>

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

void TaggerDevice::start()
{
	// initCard();
	// sendBoardInfo();
}

void TaggerDevice::stop()
{
	// closeCard();
}

void TaggerDevice::emitTaggerError()
{
	emit tagger_message(QString("TAGGER ERROR: This is some error message you may get from the tagger card: %1").arg("string"));
}

/////////////////////////////////////////////////////////////////////////////////

void TaggerDevice::initCard()
{
	// timetagger4_init_parameters * params;
	// timetagger4_get_default_init_parameters(params);

	// params->buffer_size[0] = 8 * 1024 * 1024;//8MByte packet buffer size
	// params->board_id = 0;//first board
	// params->card_index = 0;//find first board in system

	// //initialise the card
	// int error_code;
	// const char * err_message;
	// device = timetagger4_init(params, &error_code, &err_message);
	// if(error_code != CRONO_OK){
	// 	emit tagger_message(QString("TAGGER ERROR: timetagger4_init: %s").arg(err_message));
	// 	return;
	// }
	// card_running = true;
	
	// //config
	// timetagger4_configuration config;
	// timetagger4_get_default_configuration(device, config);

	// //set the config for the channels, start and stop
	// int channels=2;
	// for(int i=0; i<channels; i++){
	// 	config.channel[i].enabled = true;
	// 	config.channel[i].start = 0;
	// 	config.channel[i].stop = 30000;//recording window stops after 15us
		
	// 	//measure only falling edge
	// 	config.trigger[i+1].falling = 1;
	// 	config.trigger[i+1].rising = 0;
	// }
	// //start channel
	// config.trigger[0].falling = 1;
	// config.trigger[0].rising = 0;

	// //write this config to the board
	// int status = timetagger4_configure(device, &config);
	// if(status != CRONO_OK){
	// 	emit tagger_message(QString("TAGGER ERROR: timetagger4_configure: %s").arg(err_message));
	// 	return;
	// }
}

void TaggerDevice::sendBoardInfo()
{
	// //send some board info
	// timetagger4_static_info staticinfo;
	// status = timetagger4_get_static_info(device, staticinfo);
	// emit tagger_message(QString("Tagger: Board Serial: %d.%d").arg(staticinfo.board_serial>>24, staticinfo.board_serial&0xffffff));

	// QString message_str;
	// QTextStream message(&message_str);
	// message << "Tagger: Board configuration: TimeTagger4-";
	// switch(staticinfo.board_configuration)
	// {
	// 	case TIMETAGGER_1G_BOARDCONF:
	// 		message << "1G";
	// 	case TIMETAGGER_2G_BOARDCONF:
	// 		message << "2G";
	// 	default:
	// 		message << "unknown";
	// }
	// emit tagger_message(message.read_all());

	// emit tagger_message(QString("Tagger: Board Revision: %d").arg(staticinfo.board_revision));
	// emit tagger_message(QString("Tagger: Firmware Revision: %d.%d").arg(staticinfo.firmware_revision, staticinfo.subversion_revision));
	// emit tagger_message(QString("Tagger: Driver Revision: %d.%d.%d.%d").arg(((staticinfo.driver_revision>>24)&255), ((staticinfo.driver_revision>>16)&255), ((staticinfo.driver_revision>>8)&255), (staticinfo.driver_revision&255)));

	// timetagger4_param_info parinfo;
	// timetagger4_get_param_info(device, &parinfo);
	// emit tagger_message(QString("Tagger: TDC binsize: %0.2f ps").arg(parinfo.binsize));

}

void TaggerDevice::closeCard()
{
	// timetagger4_close(device);
	// card_running = false;
}