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
#include "include/RemoteHeinzingerPS.h"
#include "include/HeinzingerVoltageWindow.h"

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
	void keepHeinzingerVoltage(qreal voltage);
	void setHeinzingerVoltage(uint voltage);

	//widgets
	void setupWidget(CascWidget * widget, QAction * action);
	
	void toggleTof();
	void toggleMessage();
	void toggleHeinzinger();
	

	//devices
	void toggleDevice(QString device, bool start);
	void setupDevice(CascDevice * device, DeviceButton * button, QThread * thread);
	
	void toggleListener(bool start);
	void toggleFakePdlDevice(bool start);
	void toggleFakeTaggerDevice(bool start);
	void toggleTaggerDevice(bool start);
	void toggleHeinzingerDevice(bool start);

	void setStatusValue(qreal value);

signals:
	void new_message(QString message);
	
	void new_heinzinger_true_voltage(qreal voltage);
	void new_heinzinger_set_voltage(uint voltage);

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
	const QString fake_tagger_temp_path = "./temp/fake_tag_temp.dat";
	const QString tagger_temp_path = "./temp/tag_temp.dat";
	const QString fake_pdl_temp_path = "./temp/fake_pdl_temp.dat";
	const QString heinzinger_temp_path = "./temp/heinzinger_temp.dat";

	QMutex fakeTaggerFileMutex;
	QMutex fakePdlFileMutex;
	QMutex heinzingerFileMutex;

	//graph widget as central of main window
	GenericGraph *centralGraph;

	//task widgets
	QAction *tofAct;
	TofHistogram *tofHist;
	bool tofHist_open;

	QAction *messageAct;
	MessageWindow * messageWindow;
	bool messageWindow_open;
	
	QAction *heinzingerAct;
	HeinzingerVoltageWindow * heinzingerWindow;
	bool heinzingerWindow_open;
	uint maxHeinzingerVoltage;

	//devices
	DeviceButton * listenerButton;
	Listener * listener;
	bool listener_running;

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
	
	DeviceButton * heinzingerDeviceButton;
	// HeinzingerPS * heinzingerDevice;
	bool heinzinger_started;
	QThread heinzingerDeviceThread;
};

#endif //MAIN_WINDOW_H