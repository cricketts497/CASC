#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "include/PdlScanner.h"
#include "include/DeviceButton.h"
#include "include/PdlDevice.h"
#include "include/GenericGraph.h"
#include "include/fakeTagger.h"
#include "include/TofHistogram.h"
#include "include/MessageWindow.h"
#include "include/TaggerDevice.h"

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
	const QString ready_message = "Ready";

	const QString fake_tagger_temp_path = "./temp/fake_tag_temp.dat";
	const QString tagger_temp_path = "./temp/tag_temp.dat";
	const QString pdl_temp_path = "./temp/fake_pdl_temp.dat";
	
private slots:
	//widgets
	// void togglePdl();
	// void setStatusPDL(bool changed);
	void toggleTof();
	void toggleMessage();

	//devices
	void togglePdlDevice(bool start);
	void toggleFakeTaggerDevice(bool start);
	void toggleTaggerDevice(bool start);

	void setStatusValue(qreal value);

private:
	void createActions();
	void createStatusBar();
	void createDevicesBar();

	QToolBar *taskBar;
	QToolBar *devicesBar;
	// QWidget *mainWindow;
	// QHBoxLayout *mainLayout;

	//main status text
	QLabel *status;

	//open close actions
	// QAction *pdlAct;
	QAction *tofAct;
	QAction *messageAct;


	//graph widget as central of main window
	GenericGraph *centralGraph;


	//task widgets
	PdlScanner *pdlScanner;
	TofHistogram *tofHist;
	MessageWindow * messageWindow;

	//open close flags
	// bool PDL_open;
	bool tofHist_open;
	bool messageWindow_open;


	//devices
	DeviceButton *pdlDeviceButton;
	PdlDevice *pdlDevice;

	DeviceButton *fakeTaggerDeviceButton;
	FakeTagger *fakeTaggerDevice;
	bool fake_tagger_started;

	DeviceButton *taggerDeviceButton;
	TaggerDevice * taggerDevice;
	bool tagger_started;

};

#endif //MAIN_WINDOW_H