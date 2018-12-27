#include <QtWidgets>
#include "include/DeviceButton.h"

DeviceButton::DeviceButton(const char *name, QToolBar *parent, const char *startTip, const char *stopTip) :
QPushButton(name, parent),
started(false),
startTip(startTip),
stopTip(stopTip),
local(local)
{
	setButtonColour(closed_colour);
	setAutoFillBackground(true);
	setStatusTip(startTip);

	connect(this, &QAbstractButton::clicked, this, &DeviceButton::toggle);
}

void DeviceButton::setButtonColour(QColor colour)
{
	QPalette pal = palette();
	pal.setColor(QPalette::Button, colour);
	setPalette(pal);
	update();
}

void DeviceButton::toggle()
{
	if(started){
		emit toggle_device(false);

		setFlat(false);
		setButtonColour(closed_colour);
		started = false;
		setStatusTip(startTip);
	} else{
		emit toggle_device(true);

		setFlat(true);
		setButtonColour(ok_colour);
		started = true;
		setStatusTip(stopTip);
	}
}