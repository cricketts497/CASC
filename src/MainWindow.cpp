#include <QtWidgets>
#include <QFlags>
#include <sstream>

#include "include/MainWindow.h"

MainWindow::MainWindow() :
PDL_open(false)
{
	createActions();
	createStatusBar();
	createDevicesBar();

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

void MainWindow::createDevicesBar()
{
	devicesBar = new QToolBar("&Devices", this);

	QWidgetAction *pdlDeviceAct = new QWidgetAction(this);
	pdlDeviceButton = new QPushButton("PDL");

	//set red
	// pdlDeviceButton->setFlat(true);
	// QPalette pal = pdlDeviceButton->palette();
	// pal.setColor(QPalette::Button, QColor(Qt::red));
	// pdlDeviceButton->setAutoFillBackground(true);
	// pdlDeviceButton->setPalette(pal);
	// pdlDeviceButton->update();

	pdlDeviceAct->setDefaultWidget(pdlDeviceButton);
	pdlDeviceButton->setStatusTip("Start the PDL scanner device");
	devicesBar->addAction(pdlDeviceAct);

	addToolBar(Qt::LeftToolBarArea, devicesBar);
}

void MainWindow::togglePdl()
{
	if (PDL_open){
		mainLayout->removeWidget(pdlScanner);
		delete pdlScanner;
		pdlAct->setStatusTip("Open the PDL scanner");
		PDL_open = false;
		status->setText(ready_message);
	}else{
		pdlScanner = new PdlScanner("PDL Scanner", this);
		connect(pdlScanner, SIGNAL(valueChanged(bool)), this, SLOT(setStatusPDL(bool)));
		mainLayout->addWidget(pdlScanner, 0, Qt::AlignLeft|Qt::AlignTop);
		pdlAct->setStatusTip("Close the PDL scanner");
		PDL_open = true;
		setStatusPDL(false);
	}
}

void MainWindow::setStatusPDL(bool changed)
{
	QString message_str;
	QTextStream message(&message_str);

	//PDL
	if (PDL_open){
		if (pdlScanner->stopped){
			message << ready_message; 
		} else {
			message << "Scanning PDL";
		}

		if (pdlScanner->up_direction){
			message << " for higher wavelengths";
		} else{
			message << " for lower wavelengths";
		}

		message << ", Speed: "<< pdlScanner->speed_values[pdlScanner->currentSpeed]*pdlScanner->multiplier_values[pdlScanner->currentMultiplier]
		<< " nm/s";
	}
	
	message_str = message.readAll();
	status->setText(message_str);

	QFontMetrics fm(font());
	setMinimumWidth(fm.width(message_str)+30);
}