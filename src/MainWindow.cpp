#include "include/MainWindow.h"

MainWindow::MainWindow() :
config(new CascConfig(config_file_path, this)),
// messageWindow_open(false),
// heinzinger30kWindow_open(false),
maxHeinzinger30kVoltage(30000),
maxHeinzinger30kCurrent(3),
// heinzinger20kWindow_open(false),
maxHeinzinger20kVoltage(20000),
maxHeinzinger20kCurrent(3),
// dummyScanner_open(false),
// nxdsPumpWindow_open(false),
// agilentTV301Window_open(false),
nxdsPumpNames({"BLtest"}),
agilentTV301Names({"TurboTest"})
// listener_running(false)
// data_saver_started(false),
// heinzinger30k_started(false),
// heinzinger20k_started(false),
// nxdsPumpSet_started(false),
// agilentTV301_started(false)
{
	messages.setString(&messages_string);

	createActions();
	createStatusBar();
	createDevicesBar();
    
    QStringList filePaths = {heinzinger30k_temp_path, heinzinger20k_temp_path, nxdsPump_temp_path};
    QList<QMutex*> fileMutexes = {&heinzinger30kFileMutex, &heinzinger20kFileMutex, &nxdsPumpFileMutex};
	centralGraph = new SimpleGraph(filePaths, fileMutexes, this);
	
	setCentralWidget(centralGraph);
	
	setWindowTitle("CASC v3.0");
    setWindowIcon(QIcon("./resources/casc_logo.png"));

    connect(config, SIGNAL(config_message(QString)), this, SLOT(keepMessage(QString)));
    
	//auto start the listener
	listenerButton->click();
}

void MainWindow::createActions()
{
	//The main icon task bar to open tasks
	taskBar = addToolBar("&Tasks");

	//Error message display window
    messageAct = new CascAction("./resources/message.png", "Message window", "Open the error message display window", "Close the error message display window", taskBar);
	// const QIcon messageIcon = QIcon("./resources/message.png");
	// messageAct = new QAction(messageIcon, "&MESSAGE", this);
	// messageAct->setStatusTip("Open the error message display window");
	connect(messageAct, &QAction::triggered, this, &MainWindow::toggleMessage);
	taskBar->addAction(messageAct);
    
    dummyScannerAct = new CascAction("./resources/dummyScanner.png", "Dummy Scanner", "Open the dummy scanner for saving sets of data as scans", "Close the dummy scanner", taskBar);
    // const QIcon dummyScannerIcon = QIcon("./resources/dummyScanner.png");
	// dummyScannerAct = new QAction(dummyScannerIcon, "&DUMMY", this);
	// dummyScannerAct->setStatusTip("Open the dummy scanner for saving sets of data as scans");
	connect(dummyScannerAct, &QAction::triggered, this, &MainWindow::toggleDummyScanner);
	taskBar->addAction(dummyScannerAct);
	
    heinzinger30kAct = new CascAction("./resources/heinzinger30k.png", "Heinzinger 30k", "Open the 30kV heinzinger voltage controller", "Close the 30kV heinzinger voltage controller", taskBar);
	// const QIcon heinzinger30kIcon = QIcon("./resources/heinzinger30k.png");
	// heinzinger30kAct = new QAction(heinzinger30kIcon, "&HEINZINGER30K", this);
	// heinzinger30kAct->setStatusTip("Open the 30kV heinzinger voltage controller");
	connect(heinzinger30kAct, &QAction::triggered, this, &MainWindow::toggleHeinzinger30k);
	taskBar->addAction(heinzinger30kAct);
    
    heinzinger20kAct = new CascAction("./resources/heinzinger20k.png", "Heinzinger 20k", "Open the 20kV heinzinger voltage controller", "Close the 20kV heinzinger voltage controller", taskBar);
    // const QIcon heinzinger20kIcon = QIcon("./resources/heinzinger20k.png");
	// heinzinger20kAct = new QAction(heinzinger20kIcon, "&HEINZINGER20K", this);
	// heinzinger20kAct->setStatusTip("Open the 20kV heinzinger voltage controller");
	connect(heinzinger20kAct, &QAction::triggered, this, &MainWindow::toggleHeinzinger20k);
	taskBar->addAction(heinzinger20kAct);
    
    nxdsPumpAct = new CascAction("./resources/nxdsPump.png", "nXDS pump status viewer", "Open the nXDS pump status viewer", "Close the nXDS pump status viewer", taskBar);
    // const QIcon nxdsPumpIcon = QIcon("./resources/nxdsPump.png");
    // nxdsPumpAct = new QAction(nxdsPumpIcon, "&NXDSPUMP", this);
    // nxdsPumpAct->setStatusTip("Open the nXDS pump status viewer");
    connect(nxdsPumpAct, &QAction::triggered, this, &MainWindow::toggleNxdsPumpWindow);
    taskBar->addAction(nxdsPumpAct);
    
    agilentTV301Act = new CascAction("./resources/agilentTV301.png", "Agilent TV301 turbo monitor", "Open the Agilent TV301 turbo status viewer", "Close the Agilent TV301 turbo status viewer", taskBar);
    // const QIcon agilentTV301Icon = QIcon("./resources/agilentTV301.png");
    // agilentTV301Act = new QAction(agilentTV301Icon, "&AGILENTV301", this);
    // agilentTV301Act->setStatusTip("Open the Agilent TV301 turbo status viewer");
    connect(agilentTV301Act, &QAction::triggered, this, &MainWindow::toggleAgilentTV301Window);
    taskBar->addAction(agilentTV301Act);
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
	// connect(listenerButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleListener(bool)));
	connect(listenerButton, &QAbstractButton::clicked, this, &MainWindow::toggleListener);
    
    dataSaverDeviceButton = new DeviceButton("Data saver", devicesBar, "Start the scan saving device", "Stop the scan saving device", "DATA SAVER FAIL");
    // connect(dataSaverDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleDataSaver(bool)));
    connect(dataSaverDeviceButton, &QAbstractButton::clicked, this, &MainWindow::toggleDataSaver);

	heinzinger30kDeviceButton = new DeviceButton("Heinzinger 30kV", devicesBar, "Start the heinzinger 30kV power supply device", "Stop the heinzinger 30kV device", "HEINZINGER 30K FAIL");
	// connect(heinzinger30kDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleHeinzinger30kDevice(bool)));
	connect(heinzinger30kDeviceButton, &QAbstractButton::clicked, this, &MainWindow::toggleHeinzinger30kDevice);

    heinzinger20kDeviceButton = new DeviceButton("Heinzinger 20kV", devicesBar, "Start the heinzinger 20kV power supply device", "Stop the heinzinger 20kV device", "HEINZINGER 20K FAIL");
	// connect(heinzinger20kDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleHeinzinger20kDevice(bool)));
	connect(heinzinger20kDeviceButton, &QAbstractButton::clicked, this, &MainWindow::toggleHeinzinger20kDevice);

    nxdsPumpDeviceButton = new DeviceButton("NXDS test", devicesBar, "Start the Edwards NXDS backing pump device", "Stop the NXDS pump device", "NXDS PUMP FAIL");
    // connect(nxdsPumpDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleNxdsPumpDevice(bool)));
    connect(nxdsPumpDeviceButton, &QAbstractButton::clicked, this, &MainWindow::toggleNxdsPumpDevice);

    agilentTV301DeviceButton = new DeviceButton("Agilent test", devicesBar, "Start the Agilent TV301 Navigator turbo pump device", "Stop the Agilent TV 301 device", "AGILENT TV301 FAIL");
    // connect(agilentTV301DeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(toggleAgilentTV301Device(bool)));
    connect(agilentTV301DeviceButton, &QAbstractButton::clicked, this, &MainWindow::toggleAgilentTV301Device);

    //////////////////////////////////////////////////////////////////////////////////////////////
	devicesBar->addWidget(listenerButton);
    devicesBar->addWidget(dataSaverDeviceButton);
	devicesBar->addWidget(heinzinger30kDeviceButton);
	devicesBar->addWidget(heinzinger20kDeviceButton);
    devicesBar->addWidget(nxdsPumpDeviceButton);
    devicesBar->addWidget(agilentTV301DeviceButton);
    //////////////////////////////////////////////////////////////////////////////////////////////

	addToolBar(Qt::LeftToolBarArea, devicesBar);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//widgets
void MainWindow::toggleMessage()
{
	// if(messageWindow_open){
	if(messageAct->widgetToggle()){
		delete messageWindow;

		// messageAct->setStatusTip("Open the error message display window");
		// status->setText(ready_message);
		// messageWindow_open = false;
	}else{
		messageWindow = new MessageWindow(this);

		setupWidget(messageWindow, messageAct);

		//read all the stored messages
		qint64 max_message_chars = 1000;
		if(messages.pos() > max_message_chars){
			messages.seek(messages.pos()-max_message_chars);
			messageWindow->addMessage(messages.read(max_message_chars));
		}else{
            messages.seek(0);
			messageWindow->addMessage(messages.readAll());
		}
		connect(this, SIGNAL(new_message(QString)), messageWindow, SLOT(addMessage(QString)));

		addDockWidget(Qt::LeftDockWidgetArea, messageWindow);

		// messageAct->setStatusTip("Close the error message display window");
		// messageWindow_open = true;
	}
}

void MainWindow::keepMessage(QString message)
{
	messages << message;
	messages << endl;
	emit new_message(message);
    messages.seek(messages.pos()+message.length()+1);
}

	
void MainWindow::toggleHeinzinger30k()
{
	// if(heinzinger30kWindow_open){
	if(heinzinger30kAct->widgetToggle()){
		delete heinzinger30kWindow;
		
		// heinzinger30kAct->setStatusTip("Open the 30kV heinzinger voltage controller");
		// heinzinger30kWindow_open = false;
	}else{
		heinzinger30kWindow = new HeinzingerVoltageWindow(heinzinger30k_temp_path, &heinzinger30kFileMutex, maxHeinzinger30kVoltage, maxHeinzinger30kCurrent, this);
		setupWidget(heinzinger30kWindow, heinzinger30kAct);
		
		// connect(heinzinger30kWindow, SIGNAL(sendCommand(QString)), this, SLOT(heinzinger30kCommand(QString)));
		// connect(this, SIGNAL(newHeinzinger30kStatus(QString)), heinzinger30kWindow, SLOT(receiveHeinzingerStatus(QString)));
		connect(heinzinger30kDeviceButton, SIGNAL(newDeviceStatus(QString)), heinzinger30kWindow, SLOT(receiveHeinzingerStatus(QString)));
        
        if(heinzinger30kDeviceButton->deviceIsRunning())
            heinzinger30kWindow->heinzingerDeviceOn(true);
        
		addDockWidget(Qt::RightDockWidgetArea, heinzinger30kWindow);
		
		// heinzinger30kAct->setStatusTip("Close the 30kV heinzinger voltage controller");
		// heinzinger30kWindow_open = true;
	}
}
// void MainWindow::heinzinger30kCommand(QString command)
// {
    // emit newHeinzinger30kCommand(command);
// }

void MainWindow::toggleHeinzinger20k()
{
	// if(heinzinger20kWindow_open){
	if(heinzinger20kAct->widgetToggle()){
		delete heinzinger20kWindow;
		
		// heinzinger20kAct->setStatusTip("Open the 20kV heinzinger voltage controller");
		// heinzinger20kWindow_open = false;
	}else{
		heinzinger20kWindow = new HeinzingerVoltageWindow(heinzinger20k_temp_path, &heinzinger20kFileMutex, maxHeinzinger20kVoltage, maxHeinzinger20kCurrent, this);
		setupWidget(heinzinger20kWindow, heinzinger20kAct);
		
		// connect(heinzinger20kWindow, SIGNAL(sendCommand(QString)), this, SLOT(heinzinger20kCommand(QString)));
        // connect(this, SIGNAL(newHeinzinger20kStatus(QString)), heinzinger20kWindow, SLOT(receiveHeinzingerStatus(QString)));
        connect(heinzinger20kDeviceButton, SIGNAL(newDeviceStatus(QString)), heinzinger20kWindow, SLOT(receiveHeinzingerStatus(QString)));
		
        if(heinzinger20kDeviceButton->deviceIsRunning())
            heinzinger20kWindow->heinzingerDeviceOn(true);
        
		addDockWidget(Qt::BottomDockWidgetArea, heinzinger20kWindow);
		
		// heinzinger20kAct->setStatusTip("Close the 20kV heinzinger voltage controller");
		// heinzinger20kWindow_open = true;
	}
}
// void MainWindow::heinzinger20kCommand(QString command)
// {
    // emit newHeinzinger20kCommand(command);
// }


void MainWindow::toggleDummyScanner()
{
    // if(dummyScanner_open){
    if(dummyScannerAct->widgetToggle()){
        delete dummyScanner;
        
        // dummyScannerAct->setStatusTip("Open the dummy scanner");
        // dummyScanner_open = false;
    }else{
        dummyScanner = new DummyScanner(this);
        setupWidget(dummyScanner, dummyScannerAct);
    
        // connect(dummyScanner, SIGNAL(sendCommand(QString)), this, SLOT(dummyScannerCommand(QString)));
        
        if(dataSaverDeviceButton->deviceIsRunning()){
            dummyScanner->dataSaverOn(true);
        }
        
        addDockWidget(Qt::TopDockWidgetArea, dummyScanner);
        
        // dummyScannerAct->setStatusTip("Close the dummy scanner");
        // dummyScanner_open = true;
    }    
}

// void MainWindow::dummyScannerCommand(QString command)
// {
    // emit newDummyScannerCommand(command);
// }

void MainWindow::toggleNxdsPumpWindow()
{
    // if(nxdsPumpWindow_open){
    if(nxdsPumpAct->widgetToggle()){
        delete nxdsPumpWindow;
        
        // nxdsPumpAct->setStatusTip("Open the nXDS pump status viewer");
        // nxdsPumpWindow_open = false;
    }else{
        nxdsPumpWindow = new NxdsPumpStatusWindow(nxdsPumpNames, this);
        setupWidget(nxdsPumpWindow, nxdsPumpAct);
        
        // connect(this, SIGNAL(newNxdsPumpStatus(QString)), nxdsPumpWindow, SLOT(receiveNxdsStatus(QString)));
        connect(nxdsPumpDeviceButton, SIGNAL(newDeviceStatus(QString)), nxdsPumpWindow, SLOT(receiveNxdsStatus(QString)));
        
        addDockWidget(Qt::TopDockWidgetArea, nxdsPumpWindow);
        
        // nxdsPumpAct->setStatusTip("Close the nXDS pump status viewer");
        // nxdsPumpWindow_open = true;
        
        //tests
        //pump off
        // emit newNxdsPumpStatus(QString("Status_BLtest_0;0400;0000;0000;0000_42"));
        //pump running with service due, ignored
        // emit newNxdsPumpStatus(QString("Status_BLtest_30;047A;0010;0000;0000_42"));
        //decel with one of the random higher bits
        // emit newNxdsPumpStatus(QString("Status_BLtest_24;0441;8000;0000;0000_42"));
        //decel with high pump controller temperature warning
        // emit newNxdsPumpStatus(QString("Status_BLtest_24;0441;0040;0400;0000_-200"));
        //pump off with low pump controller temperature fault
        // emit newNxdsPumpStatus(QString("Status_BLtest_0;0400;0080;0000;0010_50"));
    }        
}

void MainWindow::toggleAgilentTV301Window()
{
    // if(agilentTV301Window_open){
    if(agilentTV301Act->widgetToggle()){
        delete agilentTV301Window;
        
        // agilentTV301Act->setStatusTip("Open the Agilent TV301 turbo status viewer");
        // agilentTV301Window_open = false;
    }else{
        agilentTV301Window = new AgilentTV301StatusWindow(agilentTV301Names, this);
        setupWidget(agilentTV301Window, agilentTV301Act);
        
        // connect(this, SIGNAL(newAgilentTV301Status(QString)), agilentTV301Window, SLOT(receiveAgilentTV301Status(QString)));
        connect(agilentTV301DeviceButton, SIGNAL(newDeviceStatus(QString)), agilentTV301Window, SLOT(receiveAgilentTV301Status(QString)));
        
        addDockWidget(Qt::LeftDockWidgetArea, agilentTV301Window);
        
        // agilentTV301Act->setStatusTip("Close the Agilent TV301 turbo status viewer");
        // agilentTV301Window_open = true;
    }    
    
    //tests
    //pump off
    // emit newAgilentTV301Status(QString("Status_TurboTest_0_0_40_0"));
    //Normal
    // emit newAgilentTV301Status(QString("Status_TurboTest_5_0_30.4_963"));
    //high pump temperature fault
    // emit newAgilentTV301Status(QString("Status_TurboTest_6_2_30.4_900"));
    //Too high load fault
    // emit newAgilentTV301Status(QString("Status_TurboTest_6_128_30.4_850"));
}


/////////////////////////////////////////

void MainWindow::setupWidget(CascWidget * widget, CascAction * button)
{
	connect(widget, SIGNAL(closing()), button, SLOT(trigger()));
    connect(widget, SIGNAL(widget_message(QString)), this, SLOT(keepMessage(QString)));
    
    connect(widget, SIGNAL(sendCommand(QString)), button, SLOT(widgetSendingCommand(QString)));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


//devices
void MainWindow::toggleListener()
{
	if(listenerButton->deviceToggle()){
        delete listener;
        listenerButton->deviceHasStopped();
    }else{
		listener = new Listener(config);

		connect(listener, SIGNAL(listener_fail()), listenerButton, SLOT(setFail()));
		connect(listener, SIGNAL(listener_message(QString)), this, SLOT(keepMessage(QString)));
        
		connect(listener, SIGNAL(toggle_device_command(QString,bool)), this, SLOT(toggleDevice(QString, bool)));

		listener->start();
    }
}

//currently only saving permenant files for 30k heinzinger
void MainWindow::toggleDataSaver()
{
    if(dummyScannerAct->widgetIsOpen()){
        //sending before switch in deviceToggle() below
        dummyScanner->dataSaverOn(!dataSaverDeviceButton->deviceIsRunning());
    }
    
    if(dataSaverDeviceButton->deviceToggle()){
        return;
    }
    
    bool local = config->deviceLocal(QString("datasaver"));
    
    if(local){
        QStringList temp_path_list = {heinzinger30k_temp_path};
        QVector<QMutex*> file_mutex_list = {&heinzinger30kFileMutex};
        DataSaver * dataSaverDevice = new DataSaver(temp_path_list, finalBasePath, file_mutex_list, config);
        setupDevice(dataSaverDevice, dataSaverDeviceButton, &dataSaverDeviceThread);
        // connect(this, SIGNAL(newDummyScannerCommand(QString)), dataSaverDevice, SLOT(saverCommand(QString)));
        connect(dummyScannerAct, SIGNAL(newWidgetCommand(QString)), dataSaverDevice, SLOT(saverCommand(QString)));
    }else{
        RemoteDataSaver * dataSaverDevice = new RemoteDataSaver(heinzinger30kDeviceButton->deviceIsRunning(), config);
        setupDevice(dataSaverDevice, dataSaverDeviceButton, &dataSaverDeviceThread);
        // connect(this, SIGNAL(newDummyScannerCommand(QString)), dataSaverDevice, SLOT(remoteDeviceCommand(QString)));
        connect(dummyScannerAct, SIGNAL(newWidgetCommand(QString)), dataSaverDevice, SLOT(remoteDeviceCommand(QString)));
        connect(this, SIGNAL(newDataSaverStart(QString)), dataSaverDevice, SLOT(startDevice(QString)));
    }
}

void MainWindow::dataSaverStart(QString device)
{
    emit newDataSaverStart(device);
}

void MainWindow::toggleHeinzinger30kDevice()
{
    //before doing the button toggle
    if(heinzinger30kAct->widgetIsOpen())
        heinzinger30kWindow->heinzingerDeviceOn(!heinzinger30kDeviceButton->deviceIsRunning());
    
    if(heinzinger30kDeviceButton->deviceToggle()){
        return;
    }
    
    bool local = config->deviceLocal(QString("heinzingerps30k"));
    
    if(local){
        HeinzingerPS * heinzinger30kDevice = new HeinzingerPS(maxHeinzinger30kVoltage, maxHeinzinger30kCurrent, heinzinger30k_temp_path, &heinzinger30kFileMutex, QString("heinzingerps30k"), config);
        setupDevice(heinzinger30kDevice, heinzinger30kDeviceButton, &heinzinger30kDeviceThread);
        // connect(this, SIGNAL(newHeinzinger30kCommand(QString)), heinzinger30kDevice, SLOT(queueSerialCommand(QString)));
        connect(heinzinger30kAct, SIGNAL(newWidgetCommand(QString)), heinzinger30kDevice, SLOT(queueSerialCommand(QString)));
        // connect(heinzinger30kDevice, SIGNAL(device_status(QString)), this, SLOT(heinzinger30kStatus(QString)));
    }else{
        RemoteDataDevice * heinzinger30kDevice = new RemoteDataDevice(heinzinger30k_temp_path, &heinzinger30kFileMutex, QString("heinzingerps30k"), config);
        setupDevice(heinzinger30kDevice, heinzinger30kDeviceButton, &heinzinger30kDeviceThread);
        // connect(this, SIGNAL(newHeinzinger30kCommand(QString)), heinzinger30kDevice, SLOT(remoteDeviceCommand(QString)));
        connect(heinzinger30kAct, SIGNAL(newWidgetCommand(QString)), heinzinger30kDevice, SLOT(remoteDeviceCommand(QString)));
        // connect(heinzinger30kDevice, SIGNAL(device_status(QString)), this, SLOT(heinzinger30kStatus(QString)));
    }
    //tell the data saver PC to start requesting new heinzingerps data
    dataSaverStart("heinzingerps30k");
    // heinzinger30k_started = true;
    // centralGraph->newData();
}

// void MainWindow::heinzinger30kStatus(QString status)
// {
    // emit newHeinzinger30kStatus(status);
// }

void MainWindow::toggleHeinzinger20kDevice()
{
    //before doing the button toggle
    if(heinzinger20kAct->widgetIsOpen())
        heinzinger20kWindow->heinzingerDeviceOn(!heinzinger20kDeviceButton->deviceIsRunning());
    
    if(heinzinger20kDeviceButton->deviceToggle()){
        return;
    }
    
    bool local = config->deviceLocal(QString("heinzingerps20k"));
    
    if(local){
        HeinzingerPS * heinzinger20kDevice = new HeinzingerPS(maxHeinzinger20kVoltage, maxHeinzinger20kCurrent, heinzinger20k_temp_path, &heinzinger20kFileMutex, QString("heinzingerps20k"), config);
        setupDevice(heinzinger20kDevice, heinzinger20kDeviceButton, &heinzinger20kDeviceThread);
        // connect(this, SIGNAL(newHeinzinger20kCommand(QString)), heinzinger20kDevice, SLOT(queueSerialCommand(QString)));
        connect(heinzinger20kAct, SIGNAL(newWidgetCommand(QString)), heinzinger20kDevice, SLOT(queueSerialCommand(QString)));
        // connect(heinzinger20kDevice, SIGNAL(device_status(QString)), this, SLOT(heinzinger20kStatus(QString)));
    }else{
        RemoteDataDevice * heinzinger20kDevice = new RemoteDataDevice(heinzinger20k_temp_path, &heinzinger20kFileMutex, QString("heinzingerps20k"), config);
        setupDevice(heinzinger20kDevice, heinzinger20kDeviceButton, &heinzinger20kDeviceThread);
        // connect(this, SIGNAL(newHeinzinger20kCommand(QString)), heinzinger20kDevice, SLOT(remoteDeviceCommand(QString)));
        connect(heinzinger20kAct, SIGNAL(newWidgetCommand(QString)), heinzinger20kDevice, SLOT(remoteDeviceCommand(QString)));
        // connect(heinzinger20kDevice, SIGNAL(device_status(QString)), this, SLOT(heinzinger20kStatus(QString)));
    }
    //tell the data saver PC to start requesting new heinzingerps data
    dataSaverStart("heinzingerps20k");
    // heinzinger20k_started = true;
    // centralGraph->newHeinzinger20k();
}

// void MainWindow::heinzinger20kStatus(QString status)
// {
    // emit newHeinzinger20kStatus(status);
// }

//currently have a single thread for all the backing pumps but should be ok
void MainWindow::toggleNxdsPumpDevice()
{
    if(nxdsPumpDeviceButton->deviceToggle()){
        return;
    }
    
    for(int i=0; i<nxdsPumpNames.size(); i++){
        QString dev_name = nxdsPumpNames.at(i);

        bool local = config->deviceLocal(dev_name);
        
        if(local){
            NxdsPump * nxdsDevice = new NxdsPump(nxdsPump_temp_path,&nxdsPumpFileMutex,dev_name,config);
            setupDevice(nxdsDevice, nxdsPumpDeviceButton, &nxdsPumpDeviceThread);
        }else{
            RemoteDevice * nxdsDevice = new RemoteDevice(dev_name, config);
            setupDevice(nxdsDevice, nxdsPumpDeviceButton, &nxdsPumpDeviceThread);
        }
    }
}

// void MainWindow::nxdsPumpStatus(QString status)
// {
    // emit newNxdsPumpStatus(status);
// }

void MainWindow::toggleAgilentTV301Device()
{
    //stop_device slot connected in setupDevice() below
    if(agilentTV301DeviceButton->deviceToggle()){
        return;
    }
        
    for(int i=0; i<agilentTV301Names.size(); i++){
        QString dev_name = agilentTV301Names.at(i);

        bool local = config->deviceLocal(dev_name);

        if(local){
            AgilentTV301Pump * agilentTV301Device = new AgilentTV301Pump(agilentTV301_temp_path,&agilentTV301FileMutex,dev_name,config);
            setupDevice(agilentTV301Device, agilentTV301DeviceButton, &agilentTV301DeviceThread);
        }else{
            RemoteDevice * agilentTV301Device = new RemoteDevice(dev_name, config);
            setupDevice(agilentTV301Device, agilentTV301DeviceButton, &agilentTV301DeviceThread);
        }
    }
}

// void MainWindow::agilentTV301Status(QString status)
// {
    // emit newAgilentTV301Status(status);
// }

//////////////////////////////////////////////////////////////////////////////////////

//deal with start stop signals received by listener
void MainWindow::toggleDevice(QString device, bool start)
{
    if(device == "heinzingerps30k" && ((start && !heinzinger30kDeviceButton->deviceIsRunning()) || (!start && heinzinger30kDeviceButton->deviceIsRunning())))
        heinzinger30kDeviceButton->click();
    else if(device == "heinzingerps20k" && ((start && !heinzinger20kDeviceButton->deviceIsRunning()) || (!start && heinzinger20kDeviceButton->deviceIsRunning())))
        heinzinger20kDeviceButton->click();
    else if(device == "datasaver" && ((start && !dataSaverDeviceButton->deviceIsRunning()) || (!start && dataSaverDeviceButton->deviceIsRunning())))
        dataSaverDeviceButton->click();
    else if(device == nxdsPumpNames[0] && ((start && !nxdsPumpDeviceButton->deviceIsRunning()) || (!start && nxdsPumpDeviceButton->deviceIsRunning())))
        nxdsPumpDeviceButton->click();
    else if(device == agilentTV301Names[0] && ((start && !agilentTV301DeviceButton->deviceIsRunning()) || (!start && agilentTV301DeviceButton->deviceIsRunning())))
        agilentTV301DeviceButton->click();
}

void MainWindow::setupDevice(CascDevice * device, DeviceButton * button, QThread * thread)
{
    connect(device, SIGNAL(device_status(QString)), button, SLOT(device_status(QString)));
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
	connect(button, SIGNAL(stop_device()), device, SLOT(stop_device()));
    connect(device, SIGNAL(stopped()), button, SLOT(deviceHasStopped()));
}

