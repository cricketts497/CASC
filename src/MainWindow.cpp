#include <QtWidgets>
#include <QFlags>
#include <sstream>

#include "include/MainWindow.h"

MainWindow::MainWindow()
{
	createActions();
	createStatusBar();
	createDevicesBar();

	// mainWindow = new QWidget();
	// mainLayout = new QHBoxLayout;
	// mainWindow->setLayout(mainLayout);
	// setCentralWidget(mainWindow);

	centralGraph = new GenericGraph(this);
	setCentralWidget(centralGraph);

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

	pdlDeviceButton = new DeviceButton("PDL", devicesBar, "Start the PDL scanner device", "Stop the PDL scanner device");
	connect(pdlDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(togglePdlDevice(bool)));

	taggerDeviceButton = new DeviceButton("Tagger", devicesBar, "Start the tagger device", "Stop the tagger device");
	connect(taggerDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleTaggerDevice(bool)));

	devicesBar->addWidget(pdlDeviceButton);
	devicesBar->addWidget(taggerDeviceButton);

	addToolBar(Qt::LeftToolBarArea, devicesBar);
}


//widgets
void MainWindow::togglePdl()
{
	if (PDL_open){
		delete pdlScanner;

		pdlAct->setStatusTip("Open the PDL scanner");
		PDL_open = false;
		status->setText(ready_message);
	}else{
		if (!pdlDeviceButton->started){
			pdlDeviceButton->toggle();
		}

		pdlScanner = new PdlScanner("PDL Scanner", this);
		
		connect(pdlScanner, SIGNAL(closing(bool)), this, SLOT(togglePdl()));
		connect(pdlScanner, SIGNAL(valueChanged(bool)), this, SLOT(setStatusPDL(bool)));
		connect(pdlDevice, SIGNAL(newValue(double)), pdlScanner, SLOT(updateValue(double)));

		addDockWidget(Qt::LeftDockWidgetArea, pdlScanner);

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

		// if (pdlScanner->up_direction){
		if (pdlScanner->currentDirection == 0){
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


//devices
//only local devices atm
void MainWindow::togglePdlDevice(bool start)
{
	if(start){
		pdlDevice = new PdlDevice(1000, this);
	}else{
		delete pdlDevice;
	}
}

void MainWindow::toggleTaggerDevice(bool start)
{
	if(start){
		//1s^-1 fake rate
		taggerDevice = new FakeTagger(1, this);
		connect(taggerDevice, SIGNAL(updateHits(int)), this, SLOT(setStatusTagger(int)));
	}else{
		delete taggerDevice;
		status->setText(ready_message);
	}
}

void MainWindow::setStatusTagger(int hits)
{
	QString message_str;
	QTextStream message(&message_str);

	message << "Packet hits: " << hits;

	message_str = message.readAll();
	status->setText(message_str);

	QFontMetrics fm(font());
	setMinimumWidth(fm.width(message_str)+30);
}
