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
	void setStatus(bool changed);

private:
	void createActions();
	void createStatusBar();

	QToolBar *taskBar;
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
};

#endif //MAIN_WINDOW_H