#include <QtWidgets>
#include "include/DeviceButton.h"

DeviceButton::DeviceButton(const char *name, QWidget * parent, const char *startTip, const char *stopTip, const char * failTip) :
QPushButton(name, parent),
started(false),
startTip(startTip),
stopTip(stopTip),
failTip(failTip)
{	
	// setButtonColour(closed_colour);
	QPalette pal = palette();
	pal.setColor(QPalette::Button, closed_colour);
	setPalette(pal);
	update();
	
	setAutoFillBackground(true);
	setStatusTip(startTip);

	// connect(this, &QAbstractButton::clicked, this, &DeviceButton::toggle);
}

//returns whether the device was open or closed before the toggle
bool DeviceButton::deviceToggle()
{
	if(started){
		setFlat(false);
		
		// setButtonColour(closed_colour);
		QPalette pal = palette();
		pal.setColor(QPalette::Button, closed_colour);
		setPalette(pal);
		update();
		
		started = false;
		setStatusTip(startTip);
		
        //disable the button until the device has stopped
        setEnabled(false);
        
		// emit toggle_device(false);
        emit stop_device();
        return true;
	}else{
		setFlat(true);
		
		// setButtonColour(ok_colour);
		QPalette pal = palette();
		pal.setColor(QPalette::Button, ok_colour);
		setPalette(pal);
		update();
		
		started = true;
		setStatusTip(stopTip);
		
		// emit toggle_device(true);
        return false;
	}
}

void DeviceButton::setFail()
{
    if(!started){
        return;
    }        
	setFlat(true);
	
	// setButtonColour(fail_colour);
	QPalette pal = palette();
	pal.setColor(QPalette::Button, fail_colour);
	setPalette(pal);
	update();
	
	setStatusTip(failTip);
}

void DeviceButton::device_status(QString status)
{
    emit newDeviceStatus(status);
}

bool DeviceButton::deviceIsRunning()
{
    return started;
}

void DeviceButton::deviceHasStopped()
{
    setEnabled(true);
}



