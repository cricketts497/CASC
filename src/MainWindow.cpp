#include <QtWidgets>

#include "include/MainWindow.h"

MainWindow::MainWindow() :
config(new CascConfig(config_file_path, this)),
tofHist_open(false),
messageWindow_open(false),
heinzingerWindow_open(false),
maxHeinzingerVoltage(20000),
maxHeinzingerCurrent(3),
dummyScanner_open(false),
listener_running(false),
data_saver_started(false),
fake_tagger_started(false),
tagger_started(false),
heinzinger_started(false)
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
	
	setWindowTitle("CASC v1");
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
	
	const QIcon heinzingerIcon = QIcon("./resources/heinzinger.png");
	heinzingerAct = new QAction(heinzingerIcon, "&HEINZINGER", this);
	heinzingerAct->setStatusTip("Open the heinzinger voltage controller");
	connect(heinzingerAct, &QAction::triggered, this, &MainWindow::toggleHeinzinger);
	taskBar->addAction(heinzingerAct);
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
	
	heinzingerDeviceButton = new DeviceButton("Heinzinger", devicesBar, "Start the heinzinger power supply device", "Stop the heinzinger device", "HEINZINGER FAIL");
	connect(heinzingerDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleHeinzingerDevice(bool)));

    //////////////////////////////////////////////////////////////////////////////////////////////
	devicesBar->addWidget(listenerButton);
    devicesBar->addWidget(dataSaverDeviceButton);
	// devicesBar->addWidget(fakePdlDeviceButton);
	// devicesBar->addWidget(fakeTaggerDeviceButton);
	// devicesBar->addWidget(taggerDeviceButton);
	devicesBar->addWidget(heinzingerDeviceButton);
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

	
void MainWindow::toggleHeinzinger()
{
	if(heinzingerWindow_open){
		delete heinzingerWindow;
		
		heinzingerAct->setStatusTip("Open the heinzinger voltage controller");
		heinzingerWindow_open = false;
	}else{
		heinzingerWindow = new HeinzingerVoltageWindow(heinzinger_temp_path, &heinzingerFileMutex, maxHeinzingerVoltage, maxHeinzingerCurrent, this);
		setupWidget(heinzingerWindow, heinzingerAct);
		
		connect(heinzingerWindow, SIGNAL(sendCommand(QString)), this, SLOT(heinzingerCommand(QString)));
		
        if(heinzinger_started)
            heinzingerWindow->heinzingerDeviceOn(true);
        
		addDockWidget(Qt::RightDockWidgetArea, heinzingerWindow);
		
		heinzingerAct->setStatusTip("Close the heinzinger voltage controller");
		heinzingerWindow_open = true;
	}
}
void MainWindow::heinzingerCommand(QString command)
{
    emit newHeinzingerCommand(command);
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

void MainWindow::toggleDataSaver(bool start)
{
    if(start){
        bool local = config->deviceLocal(QString("datasaver"));
        if(local){
            QStringList temp_path_list = {heinzinger_temp_path};
            QVector<QMutex*> file_mutex_list = {&heinzingerFileMutex};
            DataSaver * dataSaverDevice = new DataSaver(temp_path_list, finalBasePath, file_mutex_list, config);
            setupDevice(dataSaverDevice, dataSaverDeviceButton, &dataSaverDeviceThread);
            connect(this, SIGNAL(newDummyScannerCommand(QString)), dataSaverDevice, SLOT(deviceCommand(QString)));
        }else{
            RemoteDataSaver * dataSaverDevice = new RemoteDataSaver(heinzinger_started, config);
            setupDevice(dataSaverDevice, dataSaverDeviceButton, &dataSaverDeviceThread);
            connect(this, SIGNAL(newDummyScannerCommand(QString)), dataSaverDevice, SLOT(deviceCommand(QString)));
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

void MainWindow::toggleHeinzingerDevice(bool start)
{
	bool local = config->deviceLocal(QString("heinzingerps"));
	
	if(start){
		if(local){
			HeinzingerPS * heinzingerDevice = new HeinzingerPS(maxHeinzingerVoltage, maxHeinzingerCurrent, heinzinger_temp_path, &heinzingerFileMutex, QString("heinzingerps"), config);
			setupDevice(heinzingerDevice, heinzingerDeviceButton, &heinzingerDeviceThread);
            connect(this, SIGNAL(newHeinzingerCommand(QString)), heinzingerDevice, SLOT(deviceCommand(QString)));
        }else{
            RemoteDataDevice * heinzingerDevice = new RemoteDataDevice(heinzinger_temp_path, &heinzingerFileMutex, QString("heinzingerps"), config);
            setupDevice(heinzingerDevice, heinzingerDeviceButton, &heinzingerDeviceThread);
            connect(this, SIGNAL(newHeinzingerCommand(QString)), heinzingerDevice, SLOT(deviceCommand(QString)));
		}
        //tell the data saver PC to start requesting new heinzingerps data
        dataSaverStart("heinzingerps");
		heinzinger_started = true;
	}else{
		//stop_device slot connection in setupDevice() below
		heinzinger_started = false;
	}
    if(heinzingerWindow_open)
        heinzingerWindow->heinzingerDeviceOn(heinzinger_started);
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
    else if(device == "heinzingerps" && ((start && !heinzinger_started && !heinzingerDeviceButton->started) || (!start && heinzinger_started && heinzingerDeviceButton->started)))
        heinzingerDeviceButton->click();
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
