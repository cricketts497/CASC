#include <QtWidgets>
#include <QFlags>

#include "include/MainWindow.h"

MainWindow::MainWindow() :
PDL_open(false)
{
	createActions();
	createStatusBar();

	mainWindow = new QWidget();
	mainLayout = new QHBoxLayout;
	mainWindow->setLayout(mainLayout);
	setCentralWidget(mainWindow);

	setWindowTitle("CASC");
}

void MainWindow::createActions()
{
	//The main icon task bar to open tasks
	taskBar = addToolBar("&Tasks");

	//PDL scanner
	const QIcon pdlIcon = QIcon("./resources/pdl.png");
	pdlAct = new QAction(pdlIcon, "&PDL", this);
	pdlAct->setStatusTip("Open the PDL scanner");
	connect(pdlAct, &QAction::triggered, this, &MainWindow::togglePdl);
	taskBar->addAction(pdlAct);
}

void MainWindow::createStatusBar()
{
	status = new QLabel(ready_message);
	statusBar()->addWidget(status);
}

void MainWindow::togglePdl()
{
	if (PDL_open){
		mainLayout->removeWidget(pdlScanner);
		delete pdlScanner;
		pdlAct->setStatusTip("Open the PDL scanner");
		PDL_open = false;
	}else{
		pdlScanner = new PdlScanner("PDL Scanner", this);
		connect(pdlScanner, SIGNAL(valueChanged(bool)), this, SLOT(setStatus(bool)));
		mainLayout->addWidget(pdlScanner);
		pdlAct->setStatusTip("Close the PDL scanner");
		PDL_open = true;
	}
}

void MainWindow::setStatus(bool changed)
{
	QString message;

	if (PDL_open){
		if (pdlScanner->stopped){
			message = ready_message;
		} else if (pdlScanner->up_direction){
			message = "Scanning PDL for higher wavelengths";
		} else{
			message = "Scanning PDL for lower wavelengths";
		}
	}

	status->setText(message);
}