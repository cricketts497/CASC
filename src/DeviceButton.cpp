#include <QtWidgets>
#include "include/DeviceButton.h"

DeviceButton::DeviceButton(const char *name, QToolBar *parent, const char *startTip, const char *stopTip, const char * failTip) :
QPushButton(name, parent),
started(false),
startTip(startTip),
stopTip(stopTip),
failTip(failTip)
// local(local)
{
	emit button_message(QString("Button: init"));
	
	// setButtonColour(closed_colour);
	QPalette pal = palette();
	pal.setColor(QPalette::Button, closed_colour);
	setPalette(pal);
	update();
	
	setAutoFillBackground(true);
	setStatusTip(startTip);

	connect(this, &QAbstractButton::clicked, this, &DeviceButton::toggle);

}

// void DeviceButton::setButtonColour(QColor colour)
// {
	// emit button_message(QString("Button: setButtonColour"));
	// QPalette pal = palette();
	// pal.setColor(QPalette::Button, colour);
	// setPalette(pal);
	// update();
// }

void DeviceButton::toggle()
{
	emit button_message(QString("Button: toggle"));
	if(started){
		setFlat(false);
		
		// setButtonColour(closed_colour);
		QPalette pal = palette();
		pal.setColor(QPalette::Button, closed_colour);
		setPalette(pal);
		update();
		
		started = false;
		setStatusTip(startTip);
		
		emit toggle_device(false);
	} else{
		setFlat(true);
		
		// setButtonColour(ok_colour);
		QPalette pal = palette();
		pal.setColor(QPalette::Button, ok_colour);
		setPalette(pal);
		update();
		
		started = true;
		setStatusTip(stopTip);
		
		emit toggle_device(true);
	}
}

void DeviceButton::setFail()
{
	emit button_message(QString("Button: setFail"));
	setFlat(true);
	
	// setButtonColour(fail_colour);
	QPalette pal = palette();
	pal.setColor(QPalette::Button, fail_colour);
	setPalette(pal);
	update();
	
	setStatusTip(failTip);
}