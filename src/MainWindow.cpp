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
	messages.setString(&messages_string);

	createActions();
	createStatusBar();
	createDevicesBar();

	centralGraph = new GenericGraph(fake_tagger_temp_path, fake_pdl_temp_path, &fakeTaggerFileMutex, &fakePdlFileMutex, this);
	// centralGraph = new GenericGraph(tagger_temp_path, pdl_temp_path, this);
	
	connect(centralGraph, SIGNAL(newEdge(qreal)), this, SLOT(setStatusValue(qreal)));
	connect(centralGraph, SIGNAL(graph_message(QString)), this, SLOT(keepMessage(QString)));
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

	listenerButton = new DeviceButton("Listener", true, devicesBar, "Start the listener", "Stop the Listener", "LISTENER FAIL");
	connect(listenerButton, SIGNAL(toggle_device(bool, bool)), this, SLOT(toggleListener(bool)));

	fakePdlDeviceButton = new DeviceButton("Fake PDL", true, devicesBar, "Start the fake PDL scanner device", "Stop the fake PDL scanner device");
	connect(fakePdlDeviceButton, SIGNAL(toggle_device(bool, bool)), this, SLOT(toggleFakePdlDevice(bool)));

	fakeTaggerDeviceButton = new DeviceButton("Fake tagger", false, devicesBar, "Start the fake tagger device", "Stop the fake tagger device", "FAKE TAGGER FAIL");
	connect(fakeTaggerDeviceButton, SIGNAL(toggle_device(bool, bool)), this, SLOT(toggleFakeTaggerDevice(bool, bool)));

	taggerDeviceButton = new DeviceButton("Tagger", true, devicesBar, "Start the tagger device", "Stop the tagger device", "TAGGER FAIL");
	connect(taggerDeviceButton, SIGNAL(toggle_device(bool, bool)), this, SLOT(toggleTaggerDevice(bool)));

	devicesBar->addWidget(listenerButton);
	devicesBar->addWidget(fakePdlDeviceButton);
	devicesBar->addWidget(fakeTaggerDeviceButton);
	devicesBar->addWidget(taggerDeviceButton);

	addToolBar(Qt::LeftToolBarArea, devicesBar);
}

void MainWindow::keepMessage(QString message)
{
	messages << message;
	messages << endl;
	emit new_message(message);
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
		tofHist = new TofHistogram(fake_tagger_temp_path, &fakeTaggerFileMutex, this);

		connect(tofHist, SIGNAL(closing(bool)), this, SLOT(toggleTof()));
		connect(tofHist, SIGNAL(value(qreal)), this, SLOT(setStatusValue(qreal)));
		connect(tofHist, SIGNAL(selectionWindow(qreal,qreal)), centralGraph, SLOT(newSelectionWindow(qreal,qreal)));
		connect(tofHist, SIGNAL(tof_message(QString)), this, SLOT(keepMessage(QString)));

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

		//read all the stored messages
		qint64 max_message_chars = 1000;
		if(messages.pos() > max_message_chars){
			messages.seek(messages.pos()-max_message_chars);
			messageWindow->addMessage(messages.read(max_message_chars));
		}else{
			messageWindow->addMessage(messages.readAll());
		}
		connect(this, SIGNAL(new_message(QString)), messageWindow, SLOT(addMessage(QString)));

		addDockWidget(Qt::LeftDockWidgetArea, messageWindow);

		messageAct->setStatusTip("Close the error message display window");
		messageWindow_open = true;
	}
}




//devices
void MainWindow::toggleListener(bool start)
{
	if(start){
		listener = new Listener();

		connect(listener, SIGNAL(listener_fail()), listenerButton, SLOT(setFail()));
		connect(listener, SIGNAL(listener_message(QString)), this, SLOT(keepMessage(QString)));

		listener->start();
	}else{
		delete listener;
	}
}

void MainWindow::toggleFakePdlDevice(bool start)
{
	if(start){
		fakePdlDevice = new PdlDevice(500, fake_pdl_temp_path, &fakePdlFileMutex);

		connect(fakePdlDevice, SIGNAL(pdl_fail()), fakePdlDeviceButton, SLOT(setFail()));
		connect(fakePdlDevice, SIGNAL(pdl_message(QString)), this, SLOT(keepMessage(QString)));

		fakePdlDevice->moveToThread(&fakePdlDeviceThread);
		connect(&fakeTaggerDeviceThread, SIGNAL(finished()), fakePdlDevice, SLOT(deleteLater()));
		fakePdlDeviceThread.start();

		centralGraph->newPdl();
	}else{
		fakePdlDeviceThread.quit();
	}
}

void MainWindow::toggleFakeTaggerDevice(bool start, bool local)
{
	if(start && local){
		//100 events per second
		fakeTaggerDevice = new FakeTagger(10, fake_tagger_temp_path, &fakeTaggerFileMutex);

		fakeTaggerDevice->moveToThread(&fakeTaggerDeviceThread);
		connect(&fakeTaggerDeviceThread, SIGNAL(finished()), fakeTaggerDevice, SLOT(deleteLater()));
		fakeTaggerDeviceThread.start();
		
		centralGraph->newTagger();
		if(tofHist_open)
			tofHist->newTagger();
		
		fake_tagger_started = true;
	}else if(!start && local){
		fakeTaggerDeviceThread.quit();
		
		fake_tagger_started = false;
	}else if(start && !local){
		//start the remote device
		listener->sendCommand(QString("start_faketagger"), QString("crfed"), 11111)
	}else{
		//stop the remote device
		listener->sendCommand(QString("stop_faketagger"), QString("crfed"), 11111)
	}
}

void MainWindow::toggleTaggerDevice(bool start)
{
	if(start){
		taggerDevice = new TaggerDevice(100, tagger_temp_path, this);
		
		connect(taggerDevice, SIGNAL(tagger_fail()), taggerDeviceButton, SLOT(setFail()));
		connect(taggerDevice, SIGNAL(tagger_message(QString)), this, SLOT(keepMessage(QString)));

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
