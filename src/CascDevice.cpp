#include "include/CascDevice.h"

/*!
    \class CascDevice
    \brief Base class for physical or virtual backend devices in CASC.
    
    Inherited by all local and remote devices, deals with emitting errors and objects common to both local and remote devices such as connection timeout and port names.
*/
/*!
    Sets up a QTimer connection_timer which signals local/remote TCP connection timeout. Records the port the listener is connected to on all PCs from the config in hostListenPort. Records the host name of the PC where the device is local in hostAddress and the port which the device is connected to in hostDevicePort from the config.
*/
  
CascDevice::CascDevice(QString deviceName, CascConfig * config, QObject * parent) :
QObject(parent),
device_name(deviceName),
connection_timer(new QTimer(this)),
noDataMessage("no data"),
device_failed(false),
connection_timeout(1000)
{
	connection_timer->setSingleShot(true);
	connection_timer->setInterval(connection_timeout);
	
	messages.setString(&messages_string);
	
	QStringList listener = config->getDevice(QString("listener"));
	if(listener.isEmpty() || listener.size() != 2){
		storeMessage(QString("%1 ERROR: Listener not found in config").arg(deviceName), true);
		return;
	}
	hostListenPort = listener.at(1).toUShort();

	QStringList device = config->getDevice(deviceName);
	if(device.size() < 3){
		storeMessage(QString("%1 ERROR: Device not found in config").arg(deviceName), true);
		return;
	}
	hostAddress = device.at(1);
	hostDevicePort = device.at(2).toUShort();
	
	storeMessage(QString("%1: started").arg(deviceName), false);
}

/*!
    Message showing the device has stopped sent to keepMessage for the message window.
*/
CascDevice::~CascDevice()
{
	emit device_message(QString("%1: stopped").arg(device_name));
}

/*!
    The stopped() signal is emitted when the device has finished executing all tasks required before de-initialisation. Connected to \l QThread::quit() in the devices thread, in turn connected to \l QObject::deleteLater() on emission of \l QThread::finished(). \overload stopped()
*/

void CascDevice::stop_device()
{
	emit stopped();
}

/*!
    Stores \a message such as error messages for the \l messageWindow during intialisation. \a fail indicates whether this is a critical error for the device, i.e. it has stopped working.
*/
void CascDevice::storeMessage(QString message, bool fail)
{
	messages << message;
	messages << endl;

	if(fail)
		device_failed = true;
}

/*!
    Sends messages stored during intialisation to the \l MainWindow::keepMessages() for the \l MessageWindow.
*/
void CascDevice::sendMessages()
{
	emit device_message(messages.readAll());
	if(device_failed)
		emit device_fail();
}
