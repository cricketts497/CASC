#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QtWidgets>

#include "include/DeviceButton.h"
#include "include/MessageWindow.h"
#include "include/Listener.h"
#include "include/CascConfig.h"
// #include "include/RemoteDataDevice.h"
#include "include/HeinzingerPS.h"
#include "include/HeinzingerVoltageWindow.h"
// #include "include/DataSaver.h"
// #include "include/DummyScanner.h"
// #include "include/RemoteDataSaver.h"
#include "include/RemoteDevice.h"
#include "include/NxdsPump.h"
#include "include/NxdsPumpStatusWindow.h"
#include "include/SimpleGraph.h"
#include "include/AgilentTV301Pump.h"
#include "include/AgilentTV301StatusWindow.h"
#include "include/CascAction.h"
#include "include/LaseLock.h"
#include "include/LaseLockStatusWindow.h"

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
    // void toggleDummyScanner();
    void toggleNxdsPumpWindow();
    void toggleAgilentTV301Window();
    void toggleLaseLockWindow();
    
    //command re-emission functions
    // void heinzinger30kCommand(QString command);
    // void heinzinger20kCommand(QString command);
    // void dummyScannerCommand(QString command);
	

	//devices
	void toggleDevice(QString device, bool start);
	void setupDevice(CascDevice * device, DeviceButton * button, QThread * thread);
	
	void toggleListener();
    // void toggleDataSaver();
	void toggleHeinzinger30kDevice();
	void toggleHeinzinger20kDevice();
    void toggleNxdsPumpDevice();
    void toggleAgilentTV301Device();
    void toggleLaseLockDevice();
    
    //status re-emission functions
    // void heinzinger30kStatus(QString status);
    // void heinzinger20kStatus(QString status);
    // void nxdsPumpStatus(QString status);
    // void agilentTV301Status(QString status);

signals:
	void new_message(QString message);
	// void newDataSaverStart(QString device);
    
    //command signals to devices
	// void newHeinzinger30kCommand(QString command);
	// void newHeinzinger20kCommand(QString command);
    // void newDummyScannerCommand(QString command);
    
    //status reemission signals
    // void newHeinzinger30kStatus(QString status);
    // void newHeinzinger20kStatus(QString status);
    // void newNxdsPumpStatus(QString status);
    // void newAgilentTV301Status(QString status);

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

	//file paths
	const QString heinzinger30k_temp_path = "./temp/heinzinger30k_temp.csv";
	const QString heinzinger20k_temp_path = "./temp/heinzinger20k_temp.csv";
    const QString nxdsPump_temp_path = "./temp/nxdsPump_temp.csv";
    const QString agilentTV301_temp_path = "./temp/agilentTV301_temp.csv";
    const QString laseLock_temp_path = "./temp/laseLock_temp.csv";
    
    //base path containing the scan directories
    const QString finalBasePath = "./data";

    //file mutexes to prevent files from being opened concurently in separate threads
	QMutex heinzinger30kFileMutex;
	QMutex heinzinger20kFileMutex;
    QMutex nxdsPumpFileMutex;
    QMutex agilentTV301FileMutex;
    QMutex laseLockFileMutex;

	//graph widget as central of main window
	SimpleGraph *centralGraph;

	//task widgets
    ////////////////////////////////////////////////////////////////////////
	CascAction * messageAct;
	// QAction *messageAct;
	MessageWindow * messageWindow;
	// bool messageWindow_open;
	
	CascAction *heinzinger30kAct;
	// QAction *heinzinger30kAct;
	HeinzingerVoltageWindow * heinzinger30kWindow;
	// bool heinzinger30kWindow_open;
	const uint maxHeinzinger30kVoltage;
    const uint maxHeinzinger30kCurrent;
    
    CascAction *heinzinger20kAct;
    // QAction *heinzinger20kAct;
	HeinzingerVoltageWindow * heinzinger20kWindow;
	// bool heinzinger20kWindow_open;
    const uint maxHeinzinger20kVoltage;
    const uint maxHeinzinger20kCurrent;
    
    // CascAction * dummyScannerAct;
    // DummyScanner * dummyScanner;
    // // bool dummyScanner_open;
    
    CascAction * nxdsPumpAct;
    // QAction * nxdsPumpAct;
    NxdsPumpStatusWindow * nxdsPumpWindow;
    // bool nxdsPumpWindow_open;
    const QStringList nxdsPumpNames;

    CascAction * agilentTV301Act;
    // QAction * agilentTV301Act;
    AgilentTV301StatusWindow * agilentTV301Window;
    // bool agilentTV301Window_open;
    const QStringList agilentTV301Names;
    
    CascAction * laseLockAct;
    LaseLockStatusWindow * laseLockWindow;    

	//devices
    ////////////////////////////////////////////////////////////////////////////
	DeviceButton * listenerButton;
	Listener * listener;
	// bool listener_running;
    
    DeviceButton * dataSaverDeviceButton;
    // bool data_saver_started;
    QThread dataSaverDeviceThread;
    void dataSaverStart(QString device);

	DeviceButton * heinzinger30kDeviceButton;
	// bool heinzinger30k_started;
	QThread heinzinger30kDeviceThread;
    
    DeviceButton * heinzinger20kDeviceButton;
	// bool heinzinger20k_started;
	QThread heinzinger20kDeviceThread;
    
    DeviceButton * nxdsPumpDeviceButton;
    // bool nxdsPumpSet_started;
    QThread nxdsPumpDeviceThread;
    
    DeviceButton * agilentTV301DeviceButton;
    // bool agilentTV301_started;
    QThread agilentTV301DeviceThread;
    
    DeviceButton * laseLockDeviceButton;
    QThread laseLockDeviceThread;
};

#endif //MAIN_WINDOW_H