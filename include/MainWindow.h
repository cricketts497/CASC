#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "include/PdlScanner.h"
#include "include/DeviceButton.h"
#include "include/PdlDevice.h"
#include "include/GenericGraph.h"
#include "include/fakeTagger.h"

class QToolBar;
class QHBoxLayout;
class QAction;
class QWidgetAction;
class QLabel;
class QPushButton;
// class QDockWidget;

class MainWindow: public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	const QString ready_message = "Ready";

	const QString tagger_temp_path = "./temp/fake_tag_temp.dat";
	const QString pdl_temp_path = "./temp/fake_pdl_temp.dat";
	
private slots:
	//widgets
	void togglePdl();
	void setStatusPDL(bool changed);

	//devices
	void togglePdlDevice(bool start);
	void toggleTaggerDevice(bool start);

	void setStatusTagger(int value);

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
	QAction *pdlAct;

	//graph widget as central of main window
	GenericGraph *centralGraph;

	//task widgets
	PdlScanner *pdlScanner;

	//open close flags
	bool PDL_open = false;

	DeviceButton *pdlDeviceButton;
	PdlDevice *pdlDevice;

	DeviceButton *taggerDeviceButton;
	FakeTagger *taggerDevice;
};

#endif //MAIN_WINDOW_H