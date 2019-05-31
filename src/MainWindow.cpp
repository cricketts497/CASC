#include <QtWidgets>

#include "include/MainWindow.h"

MainWindow::MainWindow() :
config(new CascConfig(config_file_path, this)),
tofHist_open(false),
messageWindow_open(false),
heinzinger30kWindow_open(false),
maxHeinzinger30kVoltage(30000),
maxHeinzinger30kCurrent(3),
heinzinger20kWindow_open(false),
maxHeinzinger20kVoltage(20000),
maxHeinzinger20kCurrent(3),
dummyScanner_open(false),
listener_running(false),
data_saver_started(false),
fake_tagger_started(false),
tagger_started(false),
heinzinger30k_started(false),
heinzinger20k_started(false)
{
	messages.setString(&messages_string);

	createActions();
	createStatusBar();
	createDevicesBar();

	centralGraph = new GenericGraph(fake_tagger_temp_path, fake_pdl_temp_path, heinzinger30k_temp_path, heinzinger20k_temp_path, &fakeTaggerFileMutex, &fakePdlFileMutex, &heinzinger30kFileMutex, &heinzinger20kFileMutex, this);
	// centralGraph = new GenericGraph(tagger_temp_path, pdl_temp_path, this);
	
	connect(centralGraph, SIGNAL(newEdge(qreal)), this, SLOT(setStatusValue(qreal)));
	connect(centralGraph, SIGNAL(graph_message(QString)), this, SLOT(keepMessage(QString)));
	setCentralWidget(centralGraph);
	
	setWindowTitle("CASC v2.3");
    setWindowIcon(QIcon("./resources/casc_logo.png"));

    connect(config, SIGNAL(config_message(QString)), this, SLOT(keepMessage(QString)));
    
	//auto start the listener
	listenerButton->click();
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
	// const QIcon tofIcon = QIcon("./resources/tof.png");
	// tofAct = new QAction(tofIcon, "&TOF", this);
	// tofAct->setStatusTip("Open the tagger time of flight histogram");
	// connect(tofAct, &QAction::triggered, this, &MainWindow::toggleTof);
	// taskBar->addAction(tofAct);

	//Error message display window
	const QIcon messageIcon = QIcon("./resources/message.png");
	messageAct = new QAction(messageIcon, "&MESSAGE", this);
	messageAct->setStatusTip("Open the error message display window");
	connect(messageAct, &QAction::triggered, this, &MainWindow::toggleMessage);
	taskBar->addAction(messageAct);
    
    const QIcon dummyScannerIcon = QIcon("./resources/dummyScanner.png");
	dummyScannerAct = new QAction(dummyScannerIcon, "&DUMMY", this);
	dummyScannerAct->setStatusTip("Open the dummy scanner for saving sets of data as scans");
	connect(dummyScannerAct, &QAction::triggered, this, &MainWindow::toggleDummyScanner);
	taskBar->addAction(dummyScannerAct);
	
	const QIcon heinzinger30kIcon = QIcon("./resources/heinzinger30k.png");
	heinzinger30kAct = new QAction(heinzinger30kIcon, "&HEINZINGER30K", this);
	heinzinger30kAct->setStatusTip("Open the 30kV heinzinger voltage controller");
	connect(heinzinger30kAct, &QAction::triggered, this, &MainWindow::toggleHeinzinger30k);
	taskBar->addAction(heinzinger30kAct);
    
    const QIcon heinzinger20kIcon = QIcon("./resources/heinzinger20k.png");
	heinzinger20kAct = new QAction(heinzinger20kIcon, "&HEINZINGER20K", this);
	heinzinger20kAct->setStatusTip("Open the 20kV heinzinger voltage controller");
	connect(heinzinger20kAct, &QAction::triggered, this, &MainWindow::toggleHeinzinger20k);
	taskBar->addAction(heinzinger20kAct);
}

void MainWindow::createStatusBar()
{
	status = new QLabel(ready_message);
	statusBar()->addWidget(status);
}

void MainWindow::createDevicesBar()
{
	devicesBar = new QToolBar("&Devices", this);

	listenerButton = new DeviceButton("Listener", devicesBar, "Start the listener", "Stop the Listener", "LISTENER FAIL");
	connect(listenerButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleListener(bool)));
    
    dataSaverDeviceButton = new DeviceButton("Data saver", devicesBar, "Start the scan saving device", "Stop the scan saving device", "DATA SAVER FAIL");
    connect(dataSaverDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleDataSaver(bool)));

	// fakePdlDeviceButton = new DeviceButton("Fake PDL", devicesBar, "Start the fake PDL scanner device", "Stop the fake PDL scanner device");
	// connect(fakePdlDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleFakePdlDevice(bool)));

	// fakeTaggerDeviceButton = new DeviceButton("Fake tagger", devicesBar, "Start the fake tagger device", "Stop the fake tagger device", "FAKE TAGGER FAIL");
	// connect(fakeTaggerDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleFakeTaggerDevice(bool)));

	// taggerDeviceButton = new DeviceButton("Tagger", devicesBar, "Start the tagger device", "Stop the tagger device", "TAGGER FAIL");
	// connect(taggerDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleTaggerDevice(bool)));
	
	heinzinger30kDeviceButton = new DeviceButton("Heinzinger 30kV", devicesBar, "Start the heinzinger 30kV power supply device", "Stop the heinzinger 30kV device", "HEINZINGER 30K FAIL");
	connect(heinzinger30kDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleHeinzinger30kDevice(bool)));

    heinzinger20kDeviceButton = new DeviceButton("Heinzinger 20kV", devicesBar, "Start the heinzinger 20kV power supply device", "Stop the heinzinger 20kV device", "HEINZINGER 20K FAIL");
	connect(heinzinger20kDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleHeinzinger20kDevice(bool)));

    //////////////////////////////////////////////////////////////////////////////////////////////
	devicesBar->addWidget(listenerButton);
    devicesBar->addWidget(dataSaverDeviceButton);
	// devicesBar->addWidget(fakePdlDeviceButton);
	// devicesBar->addWidget(fakeTaggerDeviceButton);
	// devicesBar->addWidget(taggerDeviceButton);
	devicesBar->addWidget(heinzinger30kDeviceButton);
	devicesBar->addWidget(heinzinger20kDeviceButton);
    //////////////////////////////////////////////////////////////////////////////////////////////

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

		setupWidget(messageWindow, messageAct);

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

void MainWindow::keepMessage(QString message)
{
	messages << message;
	messages << endl;
	emit new_message(message);
}

	
void MainWindow::toggleHeinzinger30k()
{
	if(heinzinger30kWindow_open){
		delete heinzinger30kWindow;
		
		heinzinger30kAct->setStatusTip("Open the 30kV heinzinger voltage controller");
		heinzinger30kWindow_open = false;
	}else{
		heinzinger30kWindow = new HeinzingerVoltageWindow(heinzinger30k_temp_path, &heinzinger30kFileMutex, maxHeinzinger30kVoltage, maxHeinzinger30kCurrent, this);
		setupWidget(heinzinger30kWindow, heinzinger30kAct);
		
		connect(heinzinger30kWindow, SIGNAL(sendCommand(QString)), this, SLOT(heinzinger30kCommand(QString)));
		connect(this, SIGNAL(newHeinzinger30kStatus(QString)), heinzinger30kWindow, SLOT(receiveHeinzingerStatus(QString)));
        
        if(heinzinger30k_started)
            heinzinger30kWindow->heinzingerDeviceOn(true);
        
		addDockWidget(Qt::RightDockWidgetArea, heinzinger30kWindow);
		
		heinzinger30kAct->setStatusTip("Close the 30kV heinzinger voltage controller");
		heinzinger30kWindow_open = true;
	}
}
void MainWindow::heinzinger30kCommand(QString command)
{
    emit newHeinzinger30kCommand(command);
}

void MainWindow::toggleHeinzinger20k()
{
	if(heinzinger20kWindow_open){
		delete heinzinger20kWindow;
		
		heinzinger20kAct->setStatusTip("Open the 20kV heinzinger voltage controller");
		heinzinger20kWindow_open = false;
	}else{
		heinzinger20kWindow = new HeinzingerVoltageWindow(heinzinger20k_temp_path, &heinzinger20kFileMutex, maxHeinzinger20kVoltage, maxHeinzinger20kCurrent, this);
		setupWidget(heinzinger20kWindow, heinzinger20kAct);
		
		connect(heinzinger20kWindow, SIGNAL(sendCommand(QString)), this, SLOT(heinzinger20kCommand(QString)));
        connect(this, SIGNAL(newHeinzinger20kStatus(QString)), heinzinger20kWindow, SLOT(receiveHeinzingerStatus(QString)));
		
        if(heinzinger20k_started)
            heinzinger20kWindow->heinzingerDeviceOn(true);
        
		addDockWidget(Qt::BottomDockWidgetArea, heinzinger20kWindow);
		
		heinzinger20kAct->setStatusTip("Close the 20kV heinzinger voltage controller");
		heinzinger20kWindow_open = true;
	}
}
void MainWindow::heinzinger20kCommand(QString command)
{
    emit newHeinzinger20kCommand(command);
}


void MainWindow::toggleDummyScanner()
{
    if(dummyScanner_open){
        delete dummyScanner;
        
        dummyScannerAct->setStatusTip("Open the dummy scanner");
        dummyScanner_open = false;
    }else{
        dummyScanner = new DummyScanner(this);
        setupWidget(dummyScanner, dummyScannerAct);
    
        connect(dummyScanner, SIGNAL(sendCommand(QString)), this, SLOT(dummyScannerCommand(QString)));
        
        if(data_saver_started)
            dummyScanner->dataSaverOn(true);
        
        addDockWidget(Qt::TopDockWidgetArea, dummyScanner);
        
        dummyScannerAct->setStatusTip("Close the dummy scanner");
        dummyScanner_open = true;
    }    
}
void MainWindow::dummyScannerCommand(QString command)
{
    emit newDummyScannerCommand(command);
}

/////////////////////////////////////////

void MainWindow::setupWidget(CascWidget * widget, QAction * button)
{
	connect(widget, SIGNAL(closing()), button, SLOT(trigger()));
    connect(widget, SIGNAL(widget_message(QString)), this, SLOT(keepMessage(QString)));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


//devices
void MainWindow::toggleListener(bool start)
{
	if(start){
		listener = new Listener(config);

		connect(listener, SIGNAL(listener_fail()), listenerButton, SLOT(setFail()));
		connect(listener, SIGNAL(listener_message(QString)), this, SLOT(keepMessage(QString)));
		connect(listener, SIGNAL(toggle_device_command(QString,bool)), this, SLOT(toggleDevice(QString, bool)));

		listener->start();
		listener_running = true;
	}else{
		delete listener;
		listener_running = false;
	}
}

//currently only saving permenant files for 30k heinzinger
void MainWindow::toggleDataSaver(bool start)
{
    if(start){
        bool local = config->deviceLocal(QString("datasaver"));
        if(local){
            QStringList temp_path_list = {heinzinger30k_temp_path};
            QVector<QMutex*> file_mutex_list = {&heinzinger30kFileMutex};
            DataSaver * dataSaverDevice = new DataSaver(temp_path_list, finalBasePath, file_mutex_list, config);
            setupDevice(dataSaverDevice, dataSaverDeviceButton, &dataSaverDeviceThread);
            connect(this, SIGNAL(newDummyScannerCommand(QString)), dataSaverDevice, SLOT(saverCommand(QString)));
        }else{
            RemoteDataSaver * dataSaverDevice = new RemoteDataSaver(heinzinger30k_started, config);
            setupDevice(dataSaverDevice, dataSaverDeviceButton, &dataSaverDeviceThread);
            connect(this, SIGNAL(newDummyScannerCommand(QString)), dataSaverDevice, SLOT(remoteDeviceCommand(QString)));
            connect(this, SIGNAL(newDataSaverStart(QString)), dataSaverDevice, SLOT(startDevice(QString)));
        }
        data_saver_started = true;
    }else{
        data_saver_started = false;
    }
    if(dummyScanner_open)
        dummyScanner->dataSaverOn(data_saver_started);
}

void MainWindow::dataSaverStart(QString device)
{
    emit newDataSaverStart(device);
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

void MainWindow::toggleFakeTaggerDevice(bool start)
{
	bool local = config->deviceLocal(QString("faketagger"));

	if(start){
		if(local){
			//10 events per second
			FakeTagger * fakeTaggerDevice = new FakeTagger(10, fake_tagger_temp_path, &fakeTaggerFileMutex, config);
			setupDevice(fakeTaggerDevice, fakeTaggerDeviceButton, &fakeTaggerDeviceThread);
		}else{
			RemoteDataDevice * fakeTaggerDevice = new RemoteDataDevice(fake_tagger_temp_path, &fakeTaggerFileMutex, QString("faketagger"), config);
			setupDevice(fakeTaggerDevice, fakeTaggerDeviceButton, &fakeTaggerDeviceThread);
		}

		//connect to widgets
		centralGraph->newTagger();
		if(tofHist_open)
			tofHist->newTagger();
		
		fake_tagger_started = true;
	}else{
		//stop_device slot connected in setupDevice() below
		fake_tagger_started = false;
	}
	
}

void MainWindow::toggleHeinzinger30kDevice(bool start)
{
	bool local = config->deviceLocal(QString("heinzingerps30k"));
	
	if(start){
		if(local){
			HeinzingerPS * heinzinger30kDevice = new HeinzingerPS(maxHeinzinger30kVoltage, maxHeinzinger30kCurrent, heinzinger30k_temp_path, &heinzinger30kFileMutex, QString("heinzingerps30k"), config);
			setupDevice(heinzinger30kDevice, heinzinger30kDeviceButton, &heinzinger30kDeviceThread);
            connect(this, SIGNAL(newHeinzinger30kCommand(QString)), heinzinger30kDevice, SLOT(queueSerialCommand(QString)));
            connect(heinzinger30kDevice, SIGNAL(device_status(QString)), this, SLOT(heinzinger30kStatus(QString)));
        }else{
            RemoteDataDevice * heinzinger30kDevice = new RemoteDataDevice(heinzinger30k_temp_path, &heinzinger30kFileMutex, QString("heinzingerps30k"), config);
            setupDevice(heinzinger30kDevice, heinzinger30kDeviceButton, &heinzinger30kDeviceThread);
            connect(this, SIGNAL(newHeinzinger30kCommand(QString)), heinzinger30kDevice, SLOT(remoteDeviceCommand(QString)));
            connect(heinzinger30kDevice, SIGNAL(device_status(QString)), this, SLOT(heinzinger30kStatus(QString)));
		}
        //tell the data saver PC to start requesting new heinzingerps data
        dataSaverStart("heinzingerps30k");
		heinzinger30k_started = true;
        centralGraph->newHeinzinger30k();
	}else{
		//stop_device slot connection in setupDevice() below
		heinzinger30k_started = false;
	}
    if(heinzinger30kWindow_open)
        heinzinger30kWindow->heinzingerDeviceOn(heinzinger30k_started);
}

void MainWindow::heinzinger30kStatus(QString status)
{
    emit newHeinzinger30kStatus(status);
}

void MainWindow::toggleHeinzinger20kDevice(bool start)
{
	bool local = config->deviceLocal(QString("heinzingerps20k"));
	
	if(start){
		if(local){
			HeinzingerPS * heinzinger20kDevice = new HeinzingerPS(maxHeinzinger20kVoltage, maxHeinzinger20kCurrent, heinzinger20k_temp_path, &heinzinger20kFileMutex, QString("heinzingerps20k"), config);
			setupDevice(heinzinger20kDevice, heinzinger20kDeviceButton, &heinzinger20kDeviceThread);
            connect(this, SIGNAL(newHeinzinger20kCommand(QString)), heinzinger20kDevice, SLOT(queueSerialCommand(QString)));
            connect(heinzinger20kDevice, SIGNAL(device_status(QString)), this, SLOT(heinzinger20kStatus(QString)));
        }else{
            RemoteDataDevice * heinzinger20kDevice = new RemoteDataDevice(heinzinger20k_temp_path, &heinzinger20kFileMutex, QString("heinzingerps20k"), config);
            setupDevice(heinzinger20kDevice, heinzinger20kDeviceButton, &heinzinger20kDeviceThread);
            connect(this, SIGNAL(newHeinzinger20kCommand(QString)), heinzinger20kDevice, SLOT(remoteDeviceCommand(QString)));
            connect(heinzinger20kDevice, SIGNAL(device_status(QString)), this, SLOT(heinzinger20kStatus(QString)));
		}
        //tell the data saver PC to start requesting new heinzingerps data
        dataSaverStart("heinzingerps20k");
		heinzinger20k_started = true;
        centralGraph->newHeinzinger20k();
	}else{
		//stop_device slot connection in setupDevice() below
		heinzinger20k_started = false;
	}
    if(heinzinger20kWindow_open)
        heinzinger20kWindow->heinzingerDeviceOn(heinzinger20k_started);
}

void MainWindow::heinzinger20kStatus(QString status)
{
    emit newHeinzinger20kStatus(status);
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


//////////////////////////////////////////////////////////////////////////////////////

//deal with start stop signals received by listener
void MainWindow::toggleDevice(QString device, bool start)
{
	if(device == "faketagger" && ((start && !fake_tagger_started && !fakeTaggerDeviceButton->started) || (!start && fake_tagger_started && fakeTaggerDeviceButton->started)))
		fakeTaggerDeviceButton->click();
    else if(device == "heinzingerps30k" && ((start && !heinzinger30k_started && !heinzinger30kDeviceButton->started) || (!start && heinzinger30k_started && heinzinger30kDeviceButton->started)))
        heinzinger30kDeviceButton->click();
    else if(device == "heinzingerps20k" && ((start && !heinzinger20k_started && !heinzinger20kDeviceButton->started) || (!start && heinzinger20k_started && heinzinger20kDeviceButton->started)))
        heinzinger20kDeviceButton->click();
    else if(device == "datasaver" && ((start && !data_saver_started && !dataSaverDeviceButton->started) || (!start && data_saver_started && dataSaverDeviceButton->started)))
        dataSaverDeviceButton->click();
}

void MainWindow::setupDevice(CascDevice * device, DeviceButton * button, QThread * thread)
{
	connect(device, SIGNAL(device_fail()), button, SLOT(setFail()));
	connect(device, SIGNAL(device_message(QString)), this, SLOT(keepMessage(QString)));

	//emit messages stored during init
	device->sendMessages();

	//independent thread for each device
	device->moveToThread(thread);
	connect(thread, SIGNAL(finished()), device, SLOT(deleteLater()));
	thread->start();
	connect(device, SIGNAL(stopped()), thread, SLOT(quit()));
	
	//stop the device before quitting and destroying it
	connect(button, SIGNAL(toggle_device(bool)), device, SLOT(stop_device()));
}



////////////////////////////////////////////////////////////////////////////////////////
//debug function to send messages to status bar
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
