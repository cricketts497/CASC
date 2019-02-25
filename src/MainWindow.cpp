#include <QtWidgets>
#include <QFlags>
#include <sstream>

#include "include/MainWindow.h"

MainWindow::MainWindow() :
tofHist_open(false),
// PDL_open(false),
messageWindow_open(false),
fake_tagger_started(false),
tagger_started(false)
{
	createActions();
	createStatusBar();
	createDevicesBar();

	// mainWindow = new QWidget();
	// mainLayout = new QHBoxLayout;
	// mainWindow->setLayout(mainLayout);
	// setCentralWidget(mainWindow);

	centralGraph = new GenericGraph(fake_tagger_temp_path, pdl_temp_path, this);
	// centralGraph = new GenericGraph(tagger_temp_path, pdl_temp_path, this);
	
	connect(centralGraph, SIGNAL(newEdge(qreal)), this, SLOT(setStatusValue(qreal)));
	setCentralWidget(centralGraph);
	
	setWindowTitle("CASC");
	
}

void MainWindow::createActions()
{
	//The main icon task bar to open tasks
	taskBar = addToolBar("&Tasks");

	// //PDL scanner
	// const QIcon pdlIcon = QIcon("./resources/pdl.png");
	// pdlAct = new QAction(pdlIcon, "&PDL", this);
	// pdlAct->setStatusTip("Open the PDL scanner");
	// connect(pdlAct, &QAction::triggered, this, &MainWindow::togglePdl);
	// taskBar->addAction(pdlAct);

	//TOF spectrum
	const QIcon tofIcon = QIcon("./resources/tof.png");
	tofAct = new QAction(tofIcon, "&TOF", this);
	tofAct->setStatusTip("Open the tagger time of flight histogram");
	connect(tofAct, &QAction::triggered, this, &MainWindow::toggleTof);
	taskBar->addAction(tofAct);

	//Error message display window
	const QIcon messageIcon = QIcon("./resources/message.png");
	messageAct = new QAction(messageIcon, "&MESSAGE", this);
	messageAct->setStatusTip("Open the error message display window");
	connect(messageAct, &QAction::triggered, this, &MainWindow::toggleMessage);
	taskBar->addAction(messageAct);
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

	fakeTaggerDeviceButton = new DeviceButton("Fake tagger", devicesBar, "Start the fake tagger device", "Stop the fake tagger device", "FAKE TAGGER FAIL");
	connect(fakeTaggerDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleFakeTaggerDevice(bool)));

	taggerDeviceButton = new DeviceButton("Tagger", devicesBar, "Start the tagger device", "Stop the tagger device", "TAGGER FAIL");
	connect(taggerDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleTaggerDevice(bool)));

	devicesBar->addWidget(pdlDeviceButton);
	devicesBar->addWidget(fakeTaggerDeviceButton);
	devicesBar->addWidget(taggerDeviceButton);

	addToolBar(Qt::LeftToolBarArea, devicesBar);
}


//widgets
void MainWindow::toggleTof()
{
	if(tofHist_open){
		delete tofHist;

		tofAct->setStatusTip("Open the tagger time of flight histogram");
		status->setText(ready_message);
		tofHist_open = false;
	}else{
		tofHist = new TofHistogram(fake_tagger_temp_path, this);

		connect(tofHist, SIGNAL(closing(bool)), this, SLOT(toggleTof()));
		connect(tofHist, SIGNAL(value(qreal)), this, SLOT(setStatusValue(qreal)));
		connect(tofHist, SIGNAL(selectionWindow(qreal,qreal)), centralGraph, SLOT(newSelectionWindow(qreal,qreal)));

		addDockWidget(Qt::RightDockWidgetArea, tofHist);

		//tell the histogram the tagger is running
		if(fake_tagger_started){
			tofHist->newTagger();
		}
		tofAct->setStatusTip("Close the tagger time of flight histogram");
		tofHist_open = true;
	}
}

void MainWindow::toggleMessage()
{
	if(messageWindow_open){
		delete messageWindow;

		messageAct->setStatusTip("Open the error message display window");
		status->setText(ready_message);
		messageWindow_open = false;
	}else{
		messageWindow = new MessageWindow(this);

		connect(messageWindow, SIGNAL(closing()), this, SLOT(toggleMessage()));
		connect(centralGraph, SIGNAL(graph_message(QString)), messageWindow, SLOT(addMessage(QString)));
		
		connect(taggerDeviceButton, SIGNAL(button_message(QString)), messageWindow, SLOT(addMessage(QString)));
		
		if(tagger_started){
			connect(taggerDevice, SIGNAL(tagger_message(QString)), messageWindow, SLOT(addMessage(QString)));
		}

		addDockWidget(Qt::LeftDockWidgetArea, messageWindow);

		messageAct->setStatusTip("Close the error message display window");
		messageWindow_open = true;
	}
}




//devices
//only local devices atm
void MainWindow::togglePdlDevice(bool start)
{
	if(start){
		pdlDevice = new PdlDevice(500, pdl_temp_path, this);
		centralGraph->newPdl();
	}else{
		delete pdlDevice;
		// centralGraph->closedPdl();
	}
}

void MainWindow::toggleFakeTaggerDevice(bool start)
{
	if(start){
		//100 events per second
		fakeTaggerDevice = new FakeTagger(10, fake_tagger_temp_path, this);
		
		centralGraph->newTagger();
		if(tofHist_open)
			tofHist->newTagger();
		
		fake_tagger_started = true;
	}else{
		delete fakeTaggerDevice;
		
		fake_tagger_started = false;
	}
}

void MainWindow::toggleTaggerDevice(bool start)
{
	if(start){
		taggerDevice = new TaggerDevice(100, tagger_temp_path, this);
		
		connect(taggerDevice, SIGNAL(tagger_fail()), taggerDeviceButton, SLOT(setFail()));
		if(messageWindow_open)
			connect(taggerDevice, SIGNAL(tagger_message(QString)), messageWindow, SLOT(addMessage(QString)));
		
		tagger_started = taggerDevice->start_card();
		
		// if(tagger_started)
			// centralGraph->newTagger();
	}else{
		taggerDevice->stop_card();
		delete taggerDevice;
		tagger_started = false;
	}
}

void MainWindow::setStatusValue(qreal value)
{
	QString message_str;
	QTextStream message(&message_str);

	// message << "Packet hits: " << hits;
	message << "Value: " << value;

	message_str = message.readAll();
	status->setText(message_str);

	QFontMetrics fm(font());
	setMinimumWidth(fm.width(message_str)+30);
}


//////////////////////////////////////////////////////////////
//////////////////////old fake pdl scanner////////////////////
// void MainWindow::togglePdl()
// {
// 	if (PDL_open){
// 		delete pdlScanner;

// 		pdlAct->setStatusTip("Open the PDL scanner");
// 		PDL_open = false;
// 		status->setText(ready_message);
// 	}else{
// 		if (!pdlDeviceButton->started){
// 			pdlDeviceButton->toggle();
// 		}

// 		pdlScanner = new PdlScanner("PDL Scanner", this);
		
// 		connect(pdlScanner, SIGNAL(closing(bool)), this, SLOT(togglePdl()));
// 		connect(pdlScanner, SIGNAL(valueChanged(bool)), this, SLOT(setStatusPDL(bool)));
// 		connect(pdlDevice, SIGNAL(newValue(double)), pdlScanner, SLOT(updateValue(double)));

// 		addDockWidget(Qt::LeftDockWidgetArea, pdlScanner);

// 		pdlAct->setStatusTip("Close the PDL scanner");
// 		PDL_open = true;
// 		setStatusPDL(false);
// 	}
// }

// void MainWindow::setStatusPDL(bool changed)
// {
// 	QString message_str;
// 	QTextStream message(&message_str);

// 	//PDL
// 	if (PDL_open){
// 		if (pdlScanner->stopped){
// 			message << ready_message; 
// 		} else {
// 			message << "Scanning PDL";
// 		}

// 		// if (pdlScanner->up_direction){
// 		if (pdlScanner->currentDirection == 0){
// 			message << " for higher wavelengths";
// 		} else{
// 			message << " for lower wavelengths";
// 		}

// 		message << ", Speed: "<< pdlScanner->speed_values[pdlScanner->currentSpeed]*pdlScanner->multiplier_values[pdlScanner->currentMultiplier]
// 		<< " nm/s";
// 	}
	
// 	message_str = message.readAll();
// 	status->setText(message_str);

// 	QFontMetrics fm(font());
// 	setMinimumWidth(fm.width(message_str)+30);
// }
