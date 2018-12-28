#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "include/PdlScanner.h"
#include "include/DeviceButton.h"
#include "include/PdlDevice.h"
#include "include/GenericGraph.h"

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
	
private slots:
	//widgets
	void togglePdl();
	void setStatusPDL(bool changed);

	//devices
	void togglePdlDevice(bool start);

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
	// QDockWidget *pdlScannerDock;
	PdlScanner *pdlScanner;

	//open close flags
	bool PDL_open = false;

	DeviceButton *pdlDeviceButton;
	PdlDevice *pdlDevice;
};

#endif //MAIN_WINDOW_H