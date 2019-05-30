#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include "include/DeviceButton.h"
#include "include/PdlDevice.h"
#include "include/GenericGraph.h"
#include "include/fakeTagger.h"
#include "include/TofHistogram.h"
#include "include/MessageWindow.h"
#include "include/TaggerDevice.h"
#include "include/Listener.h"
#include "include/CascConfig.h"
#include "include/RemoteDataDevice.h"
#include "include/HeinzingerPS.h"
#include "include/HeinzingerVoltageWindow.h"
#include "include/DataSaver.h"
#include "include/DummyScanner.h"
#include "include/RemoteDataSaver.h"

class QToolBar;
class QHBoxLayout;
class QAction;
class QWidgetAction;
class QLabel;
class QPushButton;

class MainWindow: public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

private slots:
	void keepMessage(QString message);

	//widgets
	void setupWidget(CascWidget * widget, QAction * action);
	
	void toggleTof();
	void toggleMessage();
	void toggleHeinzinger30k();
	void toggleHeinzinger20k();
    void toggleDummyScanner();
    
    void heinzinger30kCommand(QString command);
    void heinzinger20kCommand(QString command);
    void dummyScannerCommand(QString command);
	

	//devices
	void toggleDevice(QString device, bool start);
	void setupDevice(CascDevice * device, DeviceButton * button, QThread * thread);
	
	void toggleListener(bool start);
    void toggleDataSaver(bool start);
	void toggleFakePdlDevice(bool start);
	void toggleFakeTaggerDevice(bool start);
	void toggleTaggerDevice(bool start);
	void toggleHeinzinger30kDevice(bool start);
	void toggleHeinzinger20kDevice(bool start);
    
    void heinzinger30kStatus(QString status);
    void heinzinger20kStatus(QString status);

	void setStatusValue(qreal value);

signals:
	void new_message(QString message);
	void newDataSaverStart(QString device);
    
	void newHeinzinger30kCommand(QString command);
	void newHeinzinger20kCommand(QString command);
    void newDummyScannerCommand(QString command);
    
    void newHeinzinger30kStatus(QString status);
    void newHeinzinger20kStatus(QString status);

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
	const QString fake_tagger_temp_path = "./temp/faketag_temp.dat";
	const QString tagger_temp_path = "./temp/tagger_temp.dat";
	const QString fake_pdl_temp_path = "./temp/fakepdl_temp.dat";
	const QString heinzinger30k_temp_path = "./temp/heinzinger30k_temp.dat";
	const QString heinzinger20k_temp_path = "./temp/heinzinger20k_temp.dat";
    
    //base path containing the scan directories
    const QString finalBasePath = "./data";

	QMutex fakeTaggerFileMutex;
	QMutex fakePdlFileMutex;
	QMutex heinzinger30kFileMutex;
	QMutex heinzinger20kFileMutex;

	//graph widget as central of main window
	GenericGraph *centralGraph;

	//task widgets
	QAction *tofAct;
	TofHistogram *tofHist;
	bool tofHist_open;

	QAction *messageAct;
	MessageWindow * messageWindow;
	bool messageWindow_open;
	
	QAction *heinzinger30kAct;
	HeinzingerVoltageWindow * heinzinger30kWindow;
	bool heinzinger30kWindow_open;
	const uint maxHeinzinger30kVoltage;
    const uint maxHeinzinger30kCurrent;
    
    QAction *heinzinger20kAct;
	HeinzingerVoltageWindow * heinzinger20kWindow;
	bool heinzinger20kWindow_open;
    const uint maxHeinzinger20kVoltage;
    const uint maxHeinzinger20kCurrent;
    
    QAction * dummyScannerAct;
    DummyScanner * dummyScanner;
    bool dummyScanner_open;

	//devices
	DeviceButton * listenerButton;
	Listener * listener;
	bool listener_running;
    
    DeviceButton * dataSaverDeviceButton;
    bool data_saver_started;
    QThread dataSaverDeviceThread;
    void dataSaverStart(QString device);

	DeviceButton *fakePdlDeviceButton;
	PdlDevice *fakePdlDevice;
	QThread fakePdlDeviceThread;

	DeviceButton * fakeTaggerDeviceButton;
	// FakeTagger * fakeTaggerDevice;
	bool fake_tagger_started;
	QThread fakeTaggerDeviceThread;

	DeviceButton * taggerDeviceButton;
	TaggerDevice * taggerDevice;
	bool tagger_started;
	
	DeviceButton * heinzinger30kDeviceButton;
	bool heinzinger30k_started;
	QThread heinzinger30kDeviceThread;
    
    DeviceButton * heinzinger20kDeviceButton;
	bool heinzinger20k_started;
	QThread heinzinger20kDeviceThread;

};

#endif //MAIN_WINDOW_H