#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QtWidgets>

#include "include/MessageWindow.h"
#include "include/CascConfig.h"
#include "include/HeinzingerPS.h"
#include "include/HeinzingerVoltageWindow.h"
#include "include/NxdsPump.h"
#include "include/NxdsPumpStatusWindow.h"
#include "include/SimpleGraph.h"
#include "include/AgilentTV301Pump.h"
#include "include/AgilentTV301StatusWindow.h"
#include "include/CascAction.h"
#include "include/LaseLock.h"
#include "include/LaseLockStatusWindow.h"
#include "include/TcEpicsReadout.h"
#include "include/EpicsDeviceButton.h"
#include "include/VacuumReadout.h"

class MainWindow: public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

private slots:
	//widgets
	void setupWidget(CascWidget * widget, CascAction * action);
	
	void toggleMessage();
    void keepMessage(QString message);
    
	void toggleHeinzinger30k();
	void toggleHeinzinger20k();
    void toggleNxdsPumpWindow();
    void toggleAgilentTV301Window();
    void toggleLaseLockWindow();
    void toggleTcReadout();
    void toggleVacuumReadout();

	//devices
	void setupDevice(CascDevice * device, EpicsDeviceButton * button, QThread * thread);
	
	void startHeinzinger30kDevice(bool start);
	void startHeinzinger20kDevice(bool start);
    void startNxdsPumpDevice(bool start);
    void startAgilentTV301Device(bool start);
    void startLaseLockDevice(bool start);
    
    
signals:
	void new_message(QString message);

private:
	const QString config_file_path = "./config.txt";
	CascConfig * config;

	void createActions();
	void createStatusBar();
	void createDevicesBar();

	QToolBar *taskBar;
	QToolBar *devicesBar;

	//main status text
	QLabel *status;
	const QString ready_message = "Ready";

	QString messages_string;
	QTextStream messages;
    
    //base path containing the scan directories
    // const QString finalBasePath = "./data";

	//graph widget as central of main window
	SimpleGraph *centralGraph;

	//task widgets
    ////////////////////////////////////////////////////////////////////////
	CascAction * messageAct;
	MessageWindow * messageWindow;
	
	CascAction *heinzinger30kAct;
	HeinzingerVoltageWindow * heinzinger30kWindow;
	const uint maxHeinzinger30kVoltage;
    const uint maxHeinzinger30kCurrent;
    
    CascAction *heinzinger20kAct;
	HeinzingerVoltageWindow * heinzinger20kWindow;
    const uint maxHeinzinger20kVoltage;
    const uint maxHeinzinger20kCurrent;
    
    CascAction * nxdsPumpAct;
    NxdsPumpStatusWindow * nxdsPumpWindow;
    const QStringList nxdsPumpNames;

    CascAction * agilentTV301Act;
    AgilentTV301StatusWindow * agilentTV301Window;
    const QStringList agilentTV301Names;
    
    CascAction * laseLockAct;
    LaseLockStatusWindow * laseLockWindow;    
    
    CascAction * tcReadoutAct;
    TcEpicsReadout * tcReadoutWindow;
    
    CascAction * vacuumReadoutAct;
    VacuumReadout * vacuumReadoutWindow;

	//devices
    ////////////////////////////////////////////////////////////////////////////

	EpicsDeviceButton * heinzinger30kDeviceButton;
	QThread heinzinger30kDeviceThread;
    const QStringList heinzingerStatusWindows;
    const QStringList heinzingerCommandWindows;
    
    EpicsDeviceButton * heinzinger20kDeviceButton;
	QThread heinzinger20kDeviceThread;
    
    EpicsDeviceButton * nxdsPumpDeviceButton;
    QThread nxdsPumpDeviceThread;
    const QStringList nxdsPumpStatusWindows;
    
    EpicsDeviceButton * agilentTV301DeviceButton;
    QThread agilentTV301DeviceThread;
    const QStringList agilentTV301StatusWindows;
    
    EpicsDeviceButton * laseLockDeviceButton;
    QThread laseLockDeviceThread;
    const QStringList laseLockStatusWindows;
    
    // EpicsDeviceButton * testDeviceButton;
};

#endif //MAIN_WINDOW_H