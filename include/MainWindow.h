#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
// #include "include/PdlScanner.h"
#include "include/DeviceButton.h"
#include "include/PdlDevice.h"
#include "include/GenericGraph.h"
#include "include/fakeTagger.h"
#include "include/TofHistogram.h"
#include "include/MessageWindow.h"
#include "include/TaggerDevice.h"
#include "include/Listener.h"

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
	// void togglePdl();
	// void setStatusPDL(bool changed);
	void toggleTof();
	void toggleMessage();

	//devices
	void toggleListener(bool start);
	void toggleFakePdlDevice(bool start);
	void toggleFakeTaggerDevice(bool start, bool local);
	void toggleTaggerDevice(bool start);

	void setStatusValue(qreal value);

signals:
	void new_message(QString message);

private:
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

	QMutex fakeTaggerFileMutex;
	QMutex fakePdlFileMutex;

	//graph widget as central of main window
	GenericGraph *centralGraph;

	//task widgets
	// QAction *pdlAct;
	// PdlScanner *pdlScanner;
	// bool PDL_open;

	QAction *tofAct;
	TofHistogram *tofHist;
	bool tofHist_open;

	QAction *messageAct;
	MessageWindow * messageWindow;
	bool messageWindow_open;



	//devices
	DeviceButton * listenerButton;
	Listener * listener;

	DeviceButton *fakePdlDeviceButton;
	PdlDevice *fakePdlDevice;
	QThread fakePdlDeviceThread;

	DeviceButton *fakeTaggerDeviceButton;
	FakeTagger *fakeTaggerDevice;
	bool fake_tagger_started;
	QThread fakeTaggerDeviceThread;

	DeviceButton *taggerDeviceButton;
	TaggerDevice * taggerDevice;
	bool tagger_started;

};

#endif //MAIN_WINDOW_H