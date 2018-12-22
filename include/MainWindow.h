#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "include/PdlScanner.h"

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
	
private slots:
	void toggleDevicePdl();

	void togglePdl();
	void setStatusPDL(bool changed);

private:
	void createActions();
	void createStatusBar();
	void createDevicesBar();

	void setButtonColour(QPushButton *button, QColor colour);

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
	bool PDL_open = false;

	//devices
	//buttons
	QPushButton *pdlDeviceButton;
	// QPalette pdlDevicePal;

	//started/ stopped flags
	bool device_PDL_started = false;
};

#endif //MAIN_WINDOW_H