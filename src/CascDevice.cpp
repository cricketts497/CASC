#include "include/CascDevice.h"

/*!
    \class CascDevice
    \brief Base class for physical or virtual backend devices in CASC.
    
    Inherited by all local and remote devices, deals with emitting errors and objects common to both local and remote devices such as connection timeout and port names.
*/
/*!
    Sets up a QTimer connection_timer which signals local/remote TCP connection timeout. Records the port the listener is connected to on all PCs from the config in hostListenPort. Records the host name of the PC where the device is local in hostAddress and the port which the device is connected to in hostDevicePort from the config.
*/
  
CascDevice::CascDevice(QString deviceName, QObject * parent) :
QObject(parent),
device_name(deviceName),
// connection_timer(new QTimer(this)),
// noDataMessage("no data"),
// okMessage("ok"),
// failMessage("fail"),
// askStatusMessage("Status?"),
device_failed(false),
deviceStatus(QString("Status")),
// connection_timeout(3000),
broadcast_status_timer(new QTimer(this)),
broadcast_status_timeout(20)
{
	// connection_timer->setSingleShot(true);
	// connection_timer->setInterval(connection_timeout);
	
	messages.setString(&messages_string);
	
	// QStringList listener = config->getDevice(QString("listener"));
	// if(listener.isEmpty() || listener.size() != 2){
		// storeMessage(QString("DEVICE ERROR: %1: Listener not found in config").arg(deviceName), true);
		// return;
	// }
    
    // bool conv_ok;
	// hostListenPort = listener.at(1).toUShort(&conv_ok);
    // if(hostListenPort == 0 || !conv_ok){
        // storeMessage(QString("DEVICE ERROR: %1: TCP listener port is invalid in config").arg(deviceName), true);
		// return;
	// }

	// QStringList device = config->getDevice(deviceName);
	// if(device.size() < 3){
		// storeMessage(QString("DEVICE ERROR: %1: Device not found in config").arg(deviceName), true);
		// return;
	// }
    
	// hostAddress = device.at(1);
    // if(hostAddress.isEmpty()){
        // storeMessage(QString("DEVICE ERROR: %1: TCP host address is empty in config").arg(deviceName), true);
		// return;
	// }
    
	// hostDevicePort = device.at(2).toUShort(&conv_ok);
    // if(hostDevicePort == 0 || !conv_ok){
        // storeMessage(QString("DEVICE ERROR: %1: TCP device port is invalid in config").arg(deviceName), true);
		// return;
	// }
	
    connect(this, SIGNAL(device_fail()), this, SLOT(setFailed()));
    
	storeMessage(QString("%1: started").arg(deviceName), false);
    
    connect(broadcast_status_timer, SIGNAL(timeout()), this, SLOT(broadcastStatus()));
    broadcast_status_timer->setInterval(broadcast_status_timeout);
    broadcast_status_timer->start();
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

bool CascDevice::getDeviceFailed()
{
    return device_failed;
}

void CascDevice::setFailed()
{
    device_failed = true;  
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

QString CascDevice::getStatus()
{
    return deviceStatus;
}

void CascDevice::setStatus(QString status)
{
    QStringList status_list = status.split("_");
    
    if(status_list.length() < 2){
        emit device_message(QString("DEVICE ERROR: %1: setStatus()").arg(device_name));
        emit device_fail();
        return;
    }
    
    if((status_list.at(0) == QString("Status")) && (status_list.at(1) == device_name)){
        deviceStatus = status;
    }else if((status_list.at(0) == QString("Status")) != (status_list.at(1) == device_name)){
        emit device_message(QString("DEVICE ERROR: %1: setStatus()").arg(device_name));
        emit device_fail();
    }else{
        deviceStatus = QString("Status_%1_%2").arg(device_name).arg(status);    
    }
}

void CascDevice::broadcastStatus()
{
    emit device_status(deviceStatus);
}

void CascDevice::receiveWidgetCommand(QString command)
{
    emit newWidgetCommand(command);
}


















