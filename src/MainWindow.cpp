#include "include/MainWindow.h"

MainWindow::MainWindow() :
config(new CascConfig(config_file_path, this)),
maxHeinzinger30kVoltage(30000),
maxHeinzinger30kCurrent(3),
maxHeinzinger20kVoltage(20000),
maxHeinzinger20kCurrent(3),
heinzingerStatusWindows({"VoltageSetpoint","CurrentSetpoint","OutputSetpoint", "VoltageApplied"}),
heinzingerCommandWindows({"VoltageCommanded", "CurrentCommanded", "OutputCommanded"}),
nxdsPumpNames({"BL20MT","BL20Ebara","BLIR","BLDP","BLQT"}),
nxdsPumpStatusWindows({"20MT:Status","20MT:Service","20MT:Temperature","20MT:Speed","20Ebara:Status","20Ebara:Service","20Ebara:Temperature","20Ebara:Speed","IR:Status","IR:Service","IR:Temperature","IR:Speed","DP:Status","DP:Service","DP:Temperature","DP:Speed","QT:Status","QT:Service","QT:Temperature","QT:Speed"}),
agilentTV301Names({"TurboIRTop", "TurboIRBottom", "TurboDP"}),
agilentTV301StatusWindows({"IRTop:Status", "IRTop:Error", "IRTop:Temperature", "IRTop:Drive", "IRBottom:Status", "IRBottom:Error", "IRBottom:Temperature", "IRBottom:Drive", "DP:Status", "DP:Error", "DP:Temperature", "DP:Drive"}),
laseLockStatusWindows({"LockedA", "LockedB", "SearchA", "SearchB", "InClipA", "InClipB", "HoldA", "HoldB"}),
FC0StatusWindows({"State"}),
FC0CommandWindows({"StateCommanded"}),
agilisMirrorsStatusWindows({"Position1", "Position2", "Position3", "Position4", "Position5", "Position6", "Position7", "Position8"}),
agilisMirrorsCommandWindows({}),
agilisMirrorsUrgentCommandWindows({"StopCommanded", "Jog1", "Jog2", "Jog3", "Jog4", "Jog5", "Jog6", "Jog7", "Jog8"}),
powerMeterStatusWindows({"device", "device2"})
{
	messages.setString(&messages_string);

	createActions();
	createStatusBar();
	createDevicesBar();
    
	centralGraph = new SimpleGraph({}, {}, this);
	
	setCentralWidget(centralGraph);
	
	setWindowTitle("CASC v4.6");
    setWindowIcon(QIcon("./resources/casc_logo.png"));

    connect(config, SIGNAL(config_message(QString)), this, SLOT(keepMessage(QString)));
    
    //setup the server
    const QString epicsServerName = "CASCServer";

    QStringList serverDevice = config->getDevice(epicsServerName);
    QString serverHost = QString("%1.cern.ch").arg(serverDevice.at(1));
    qputenv("EPICS_CA_ADDR_LIST", serverHost.toUtf8());//add the host name of the EPICS server pc to the list of addresses the clients look at
    // qputenv("EPICS_CA_ADDR_LIST", "");
    // qputenv("EPICS_CA_NAME_SERVERS", serverHost.toUtf8());//add the host name of the EPICS server pc to the list of addresses the clients look at
    // qputenv("EPICS_CA_AUTO_ADDR_LIST", QString("NO").toUtf8());
}

void MainWindow::createActions()
{
	//The main icon task bar to open tasks
	taskBar = addToolBar("&Tasks");

	//Error message display window
    messageAct = new CascAction("./resources/message.png", "Message window", "Open the error message display window", "Close the error message display window", taskBar);
	connect(messageAct, &QAction::triggered, this, &MainWindow::toggleMessage);
	taskBar->addAction(messageAct);
	
    heinzinger30kAct = new CascAction("./resources/heinzinger30k.png", "Heinzinger 30k", "Open the 30kV heinzinger voltage controller", "Close the 30kV heinzinger voltage controller", taskBar);
	connect(heinzinger30kAct, &QAction::triggered, this, &MainWindow::toggleHeinzinger30k);
	taskBar->addAction(heinzinger30kAct);
    
    heinzinger20kAct = new CascAction("./resources/heinzinger20k.png", "Heinzinger 20k", "Open the 20kV heinzinger voltage controller", "Close the 20kV heinzinger voltage controller", taskBar);
	connect(heinzinger20kAct, &QAction::triggered, this, &MainWindow::toggleHeinzinger20k);
	taskBar->addAction(heinzinger20kAct);
    
    nxdsPumpAct = new CascAction("./resources/nxdsPump.png", "nXDS pump status viewer", "Open the nXDS pump status viewer", "Close the nXDS pump status viewer", taskBar);
    connect(nxdsPumpAct, &QAction::triggered, this, &MainWindow::toggleNxdsPumpWindow);
    taskBar->addAction(nxdsPumpAct);
    
    agilentTV301Act = new CascAction("./resources/agilentTV301.png", "Agilent TV301 turbo monitor", "Open the Agilent TV301 turbo status viewer", "Close the Agilent TV301 turbo status viewer", taskBar);
    connect(agilentTV301Act, &QAction::triggered, this, &MainWindow::toggleAgilentTV301Window);
    taskBar->addAction(agilentTV301Act);
    
    laseLockAct = new CascAction("./resources/laselock.png", "TEM LaseLock box monitor", "Open the TEM LaseLock status viewer", "Close the TEM LaseLock status viewer", taskBar);
    connect(laseLockAct, &QAction::triggered, this, &MainWindow::toggleLaseLockWindow);
    taskBar->addAction(laseLockAct);
    
    tcReadoutAct = new CascAction("./resources/tcReadout.png", "Thermocouple readout monitor", "Open the thermocouple temperature viewer", "Close the thermocouple temperature viewer", taskBar);
    connect(tcReadoutAct, &QAction::triggered, this, &MainWindow::toggleTcReadout);
    taskBar->addAction(tcReadoutAct);
    
    vacuumReadoutAct = new CascAction("./resources/vacuumReadout.png", "Pressure readout monitor", "Open the pressure viewer", "Close the pressure viewer", taskBar);
    connect(vacuumReadoutAct, &QAction::triggered, this, &MainWindow::toggleVacuumReadout);
    taskBar->addAction(vacuumReadoutAct);
    
    // FC0Act = new CascAction("./resources/FC0Window.png", "FC0 State", "Open the FC0 control window", "Close the FC0 control window", taskBar);
    // connect(FC0Act, &QAction::triggered, this, &MainWindow::toggleFC0Window);
    // taskBar->addAction(FC0Act);
    
    agilisMirrorsAct = new CascAction("./resources/agilisMirrors.png", "Remote mirror control", "Open the Agilis remote mirror control", "Close the Agilis remote mirror control", taskBar);
    connect(agilisMirrorsAct, &QAction::triggered, this, &MainWindow::toggleAgilisMirrorsWindow);
    taskBar->addAction(agilisMirrorsAct);
}

void MainWindow::createStatusBar()
{
	status = new QLabel(ready_message);
	statusBar()->addWidget(status);
}

void MainWindow::createDevicesBar()
{
	devicesBar = new QToolBar("&Devices", this);

	heinzinger30kDeviceButton = new EpicsDeviceButton("Heinzinger30k", heinzingerStatusWindows, "Start the heinzinger 30kV power supply device", "Stop the heinzinger 30kV device", "HEINZINGER 30K FAIL", config, devicesBar, heinzingerCommandWindows);
	connect(heinzinger30kDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(startHeinzinger30kDevice(bool)));

    heinzinger20kDeviceButton = new EpicsDeviceButton("Heinzinger20k", heinzingerStatusWindows, "Start the heinzinger 20kV power supply device", "Stop the heinzinger 20kV device", "HEINZINGER 20K FAIL", config, devicesBar, heinzingerCommandWindows);
	connect(heinzinger20kDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(startHeinzinger20kDevice(bool)));

    nxdsPumpDeviceButton = new EpicsDeviceButton("BL", nxdsPumpStatusWindows, "Start the Edwards nXDS backing pump device", "Stop the nXDS pump device", "NXDS PUMP FAIL", config, devicesBar);
    connect(nxdsPumpDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(startNxdsPumpDevice(bool)));

    agilentTV301DeviceButton = new EpicsDeviceButton("Turbo", agilentTV301StatusWindows, "Start the Agilent TV301 Navigator turbo pump device", "Stop the Agilent TV 301 device", "AGILENT TV301 FAIL", config, devicesBar);
    connect(agilentTV301DeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(startAgilentTV301Device(bool)));
    
    laseLockDeviceButton = new EpicsDeviceButton("Laselock", laseLockStatusWindows, "Start the TEM LaseLock box device", "Stop the TEM LaseLock box device", "LASELOCK FAIL", config, devicesBar);
    connect(laseLockDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(startLaseLockDevice(bool)));
    
    // FC0DeviceButton = new EpicsDeviceButton("FC0Servo", FC0StatusWindows, "Start the FC0 servo device", "Stop the FC0 servo device", "FC0 FAIL", config, devicesBar, FC0CommandWindows);
    // connect(FC0DeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(startFC0Device(bool)));
    
    agilisMirrorsDeviceButton = new EpicsDeviceButton("AgilisMirrors", agilisMirrorsStatusWindows, "Start the Newport Agilis Remote control mirrors", "Stop the Newport Agilis remote mirrors", "AGILIS MIRRORS FAIL", config, devicesBar, agilisMirrorsCommandWindows, agilisMirrorsUrgentCommandWindows);
    connect(agilisMirrorsDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(startAgilisMirrorsDevice(bool)));
    
    // powerMeterDeviceButton = new EpicsDeviceButton("PowerMeters", powerMeterStatusWindows, "Start the Thorlabs power meter control and monitoring", "Stop the Thorlabs power meters", "THORLABS POWER METER FAIL", config, devicesBar);
    // connect(powerMeterDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(startPowerMeterDevice(bool)));
    
    //////////////////////////////////////////////////////////////////////////////////////////////
	devicesBar->addWidget(heinzinger30kDeviceButton);
	devicesBar->addWidget(heinzinger20kDeviceButton);
    devicesBar->addWidget(nxdsPumpDeviceButton);
    devicesBar->addWidget(agilentTV301DeviceButton);
    devicesBar->addWidget(laseLockDeviceButton);
    // devicesBar->addWidget(FC0DeviceButton);
    devicesBar->addWidget(agilisMirrorsDeviceButton);
    // devicesBar->addWidget(powerMeterDeviceButton);
    //////////////////////////////////////////////////////////////////////////////////////////////

	addToolBar(Qt::LeftToolBarArea, devicesBar);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//widgets
void MainWindow::toggleMessage()
{
	if(messageAct->widgetToggle()){
		delete messageWindow;
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
        
        connect(heinzinger30kDeviceButton, SIGNAL(buttonMessage(QString)), messageWindow, SLOT(addMessage(QString)));
        connect(heinzinger20kDeviceButton, SIGNAL(buttonMessage(QString)), messageWindow, SLOT(addMessage(QString)));
        connect(nxdsPumpDeviceButton, SIGNAL(buttonMessage(QString)), messageWindow, SLOT(addMessage(QString)));
        connect(agilentTV301DeviceButton, SIGNAL(buttonMessage(QString)), messageWindow, SLOT(addMessage(QString)));
        connect(laseLockDeviceButton, SIGNAL(buttonMessage(QString)), messageWindow, SLOT(addMessage(QString)));
        connect(agilisMirrorsDeviceButton, SIGNAL(buttonMessage(QString)), messageWindow, SLOT(addMessage(QString)));
        
        connect(heinzinger20kDeviceButton, SIGNAL(widgetCommand(QString)), messageWindow, SLOT(addMessage(QString)));
        connect(heinzinger30kDeviceButton, SIGNAL(widgetCommand(QString)), messageWindow, SLOT(addMessage(QString)));

		addDockWidget(Qt::LeftDockWidgetArea, messageWindow);

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
	if(heinzinger30kAct->widgetToggle()){
		delete heinzinger30kWindow;
	}else{
		heinzinger30kWindow = new HeinzingerVoltageWindow("Heinzinger30k", maxHeinzinger30kVoltage, maxHeinzinger30kCurrent, this);
		setupWidget(heinzinger30kWindow, heinzinger30kAct);
        
		addDockWidget(Qt::BottomDockWidgetArea, heinzinger30kWindow);
	}
}

void MainWindow::toggleHeinzinger20k()
{
	if(heinzinger20kAct->widgetToggle()){
		delete heinzinger20kWindow;
	}else{
		heinzinger20kWindow = new HeinzingerVoltageWindow("Heinzinger20k", maxHeinzinger20kVoltage, maxHeinzinger20kCurrent, this);
		setupWidget(heinzinger20kWindow, heinzinger20kAct);
        
		addDockWidget(Qt::BottomDockWidgetArea, heinzinger20kWindow);
	}
}

void MainWindow::toggleNxdsPumpWindow()
{
    if(nxdsPumpAct->widgetToggle()){
        delete nxdsPumpWindow;
    }else{
        nxdsPumpWindow = new NxdsPumpStatusWindow(nxdsPumpNames, this);
        setupWidget(nxdsPumpWindow, nxdsPumpAct);

        addDockWidget(Qt::TopDockWidgetArea, nxdsPumpWindow);
        
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
    if(agilentTV301Act->widgetToggle()){
        delete agilentTV301Window;
    }else{
        agilentTV301Window = new AgilentTV301StatusWindow(agilentTV301Names, this);
        setupWidget(agilentTV301Window, agilentTV301Act);
        
        addDockWidget(Qt::TopDockWidgetArea, agilentTV301Window);

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
}

void MainWindow::toggleLaseLockWindow()
{
    if(laseLockAct->widgetToggle()){
        delete laseLockWindow;
    }else{
        laseLockWindow = new LaseLockStatusWindow(laseLockStatusWindows, this);
        setupWidget(laseLockWindow, laseLockAct);
        
        addDockWidget(Qt::RightDockWidgetArea, laseLockWindow);
    
        //tests
        //not locked
        // laseLockWindow->receiveLaseLockStatus(QString("Status_laselock_0_2_0_2_0_2_0_2"));
        //locked
        // laseLockWindow->receiveLaseLockStatus(QString("Status_laselock_1_1_1_1_1_1_1_1"));
        //not started
        // laseLockWindow->receiveLaseLockStatus(QString("Status_laselock_2"));
    }
}

void MainWindow::toggleTcReadout()
{
    if(tcReadoutAct->widgetToggle()){
        delete tcReadoutWindow;
    }else{
        tcReadoutWindow = new TcEpicsReadout(this);
        setupWidget(tcReadoutWindow, tcReadoutAct);
        
        addDockWidget(Qt::RightDockWidgetArea, tcReadoutWindow);
    }    
}

void MainWindow::toggleVacuumReadout()
{
    if(vacuumReadoutAct->widgetToggle()){
        delete vacuumReadoutWindow;
    }else{
        vacuumReadoutWindow = new VacuumReadout(config, this);
        setupWidget(vacuumReadoutWindow, vacuumReadoutAct);
        
        addDockWidget(Qt::LeftDockWidgetArea, vacuumReadoutWindow);
    }
}

void MainWindow::toggleFC0Window()
{
    if(FC0Act->widgetToggle()){
        delete FC0Window;
    }else{
        FC0Window = new FC0ServoWindow(QString("FC0Servo"), this);
        setupWidget(FC0Window, FC0Act);
        
        addDockWidget(Qt::TopDockWidgetArea, FC0Window);
    }    
}

void MainWindow::toggleAgilisMirrorsWindow()
{
    if(agilisMirrorsAct->widgetToggle()){
        delete agilisMirrorsWindow;
    }else{
        agilisMirrorsWindow = new AgilisMirrorsWindow(QString("AgilisMirrors"), 3, this);
        setupWidget(agilisMirrorsWindow, agilisMirrorsAct);
        
        addDockWidget(Qt::TopDockWidgetArea, agilisMirrorsWindow);
    }    
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
void MainWindow::startHeinzinger30kDevice(bool start)
{
    if(!start && !heinzinger30kDeviceThread.isRunning()){
        heinzinger30kDeviceButton->deviceHasStopped();
        return;
    }else if(!start || !config->deviceLocal(QString("Heinzinger30k"))){
        return;
    }
    
    HeinzingerPS * heinzinger30kDevice = new HeinzingerPS(maxHeinzinger30kVoltage, maxHeinzinger30kCurrent, QString("Heinzinger30k"), config);
    setupDevice(heinzinger30kDevice, heinzinger30kDeviceButton, &heinzinger30kDeviceThread);
    
    if(!heinzinger30kDevice->getDeviceFailed()){
        heinzinger30kDeviceButton->deviceHasStarted();
    }
}

void MainWindow::startHeinzinger20kDevice(bool start)
{
    if(!start && !heinzinger20kDeviceThread.isRunning()){
        heinzinger20kDeviceButton->deviceHasStopped();
        return;
    }else if(!start || !config->deviceLocal(QString("Heinzinger20k"))){
        return;
    }
    
    HeinzingerPS * heinzinger20kDevice = new HeinzingerPS(maxHeinzinger20kVoltage, maxHeinzinger20kCurrent, QString("Heinzinger20k"), config);
    setupDevice(heinzinger20kDevice, heinzinger20kDeviceButton, &heinzinger20kDeviceThread);
    
    if(!heinzinger20kDevice->getDeviceFailed()){
        heinzinger20kDeviceButton->deviceHasStarted();
    }
}


//currently have a single thread for all the backing pumps but should be ok
void MainWindow::startNxdsPumpDevice(bool start)
{
    if(!start && !nxdsPumpDeviceThread.isRunning()){
        nxdsPumpDeviceButton->deviceHasStopped();
        return;
    }else if(!start){
        return;
    }
    
    bool failed = false;
    bool one_local = false;
    for(int i=0; i<nxdsPumpNames.size(); i++){
        QString dev_name = nxdsPumpNames.at(i);

        if(!config->deviceLocal(dev_name)){
            continue;
        }else{
            one_local = true;
        }
        
        NxdsPump * nxdsDevice = new NxdsPump(dev_name,config);
        setupDevice(nxdsDevice, nxdsPumpDeviceButton, &nxdsPumpDeviceThread);

        if(nxdsDevice->getDeviceFailed()){
            failed = true;
        }        
    }
    
    if(!failed && one_local){
        nxdsPumpDeviceButton->deviceHasStarted();
    }
}

void MainWindow::startAgilentTV301Device(bool start)
{    
    if(!start && !agilentTV301DeviceThread.isRunning()){
        agilentTV301DeviceButton->deviceHasStopped();
        return;
    }else if(!start){
        return;
    }


    bool failed = false;
    bool one_local = false;
    for(int i=0; i<agilentTV301Names.size(); i++){
        QString dev_name = agilentTV301Names.at(i);

        if(!config->deviceLocal(dev_name)){
            continue;
        }else{
            one_local = true;
        }

        AgilentTV301Pump * agilentTV301Device = new AgilentTV301Pump(dev_name,config);
        setupDevice(agilentTV301Device, agilentTV301DeviceButton, &agilentTV301DeviceThread);

        if(agilentTV301Device->getDeviceFailed()){
            failed = true;
        }
    }
    
    if(!failed && one_local){
        agilentTV301DeviceButton->deviceHasStarted();
    }
}


void MainWindow::startLaseLockDevice(bool start)
{
    if(!start && !laseLockDeviceThread.isRunning()){
        laseLockDeviceButton->deviceHasStopped();
        return;
    //only need local devices as status variables accessed through epics
    }else if(!start || !config->deviceLocal(QString("Laselock"))){
        return;
    }
    
    LaseLock * laseLockDevice = new LaseLock(QString("Laselock"),config);
    setupDevice(laseLockDevice, laseLockDeviceButton, &laseLockDeviceThread);
        
    if(!laseLockDevice->getDeviceFailed()){
        laseLockDeviceButton->deviceHasStarted();
    } 
}

void MainWindow::startFC0Device(bool start)
{
    if(!start && !FC0DeviceThread.isRunning()){
        FC0DeviceButton->deviceHasStopped();
        return;
    }else if(!start || !config->deviceLocal(QString("FC0Servo"))){
        return;
    }
    
    FC0Servo * FC0ServoDevice = new FC0Servo(QString("FC0Servo"), config);
    setupDevice(FC0ServoDevice, FC0DeviceButton, &FC0DeviceThread);
    
    if(!FC0ServoDevice->getDeviceFailed()){
        FC0DeviceButton->deviceHasStarted();
    }
}

void MainWindow::startAgilisMirrorsDevice(bool start)
{
    if(!start && !agilisMirrorsDeviceThread.isRunning()){
        agilisMirrorsDeviceButton->deviceHasStopped();
        return;
    }else if(!start || !config->deviceLocal(QString("AgilisMirrors"))){
        return;
    }
    
    AgilisMirrors * agilisMirrorsDevice = new AgilisMirrors(QString("AgilisMirrors"), config);
    setupDevice(agilisMirrorsDevice, agilisMirrorsDeviceButton, &agilisMirrorsDeviceThread);
    
    if(!agilisMirrorsDevice->getDeviceFailed()){
        agilisMirrorsDeviceButton->deviceHasStarted();
    }     
}

void MainWindow::startPowerMeterDevice(bool start)
{
    if(!start && !powerMeterDeviceThread.isRunning()){
        powerMeterDeviceButton->deviceHasStopped();
        return;
    }else if(!start || !config->deviceLocal(QString("PowerMeters"))){
        return;
    }
    
    ThorlabsPowerMeters * powerMeterDevice = new ThorlabsPowerMeters(QString("PowerMeters"));
    setupDevice(powerMeterDevice, powerMeterDeviceButton, &powerMeterDeviceThread);
    
    if(!powerMeterDevice->getDeviceFailed()){
        powerMeterDeviceButton->deviceHasStarted();
    }
}

//////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setupDevice(CascDevice * device, EpicsDeviceButton * button, QThread * thread)
{
    //////
    connect(button, SIGNAL(widgetCommand(QString)), device, SLOT(receiveWidgetCommand(QString)));
    connect(button, SIGNAL(urgentWidgetCommand(QString)), device, SLOT(receiveUrgentWidgetCommand(QString)));
    //////
    connect(device, SIGNAL(device_status(QString)), button, SLOT(device_status(QString)));
    
	connect(device, SIGNAL(device_fail()), button, SLOT(setFail()));
	connect(device, SIGNAL(device_message(QString)), this, SLOT(keepMessage(QString)));

	//emit messages stored during init
	device->sendMessages();

	//independent thread for each device
	device->moveToThread(thread);
	connect(thread, SIGNAL(finished()), device, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), button, SLOT(deviceHasStopped()));
	thread->start();
	connect(device, SIGNAL(stopped()), thread, SLOT(quit()));
    
	//stop the device before quitting and destroying it
	connect(button, SIGNAL(toggle_device(bool)), device, SLOT(stop_device()));
    
    // connect(device, SIGNAL(stopped()), button, SLOT(deviceHasStopped()));
    // connect(device, SIGNAL(stopped()), device, SLOT(deleteLater()));
    
}

