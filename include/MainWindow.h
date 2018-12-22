#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "include/PdlScanner.h"

class QToolBar;
class QHBoxLayout;
class QAction;
class QLabel;

class MainWindow: public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	const QString ready_message = "Ready";
	
private slots:
	void togglePdl();
	void setStatusPDL(bool changed);

private:
	void createActions();
	void createStatusBar();
	void createDevicesBar();

	QToolBar *taskBar;
	QToolBar *devicesBar;
	QWidget *mainWindow;
	QHBoxLayout *mainLayout;

	//main status text
	QLabel *status;

	//open close actions
	QAction *pdlAct;

	//task widgets
	PdlScanner *pdlScanner;

	//open close flags
	bool PDL_open;

	//devices
	//start/stop actions
	QAction *pdlDeviceAct;

	//buttons
	QPushButton *pdlDeviceButton;
};

#endif //MAIN_WINDOW_H