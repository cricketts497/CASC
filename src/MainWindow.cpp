#include <QtWidgets>
#include <QFlags>
#include <sstream>

#include "include/MainWindow.h"

MainWindow::MainWindow()
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

	pdlDeviceButton = new DeviceButton("PDL", devicesBar, "Start the PDL scanner device", "Stop the PDL scanner device");
	connect(pdlDeviceButton, SIGNAL(toggle_device(bool)), this, SLOT(togglePdlDevice(bool)));

	devicesBar->addWidget(pdlDeviceButton);

	addToolBar(Qt::LeftToolBarArea, devicesBar);
}


//widgets
void MainWindow::togglePdl()
{
	if (PDL_open){
		mainLayout->removeWidget(pdlScanner);
		delete pdlScanner;
		pdlAct->setStatusTip("Open the PDL scanner");
		PDL_open = false;
		status->setText(ready_message);
	}else{
		if (!pdlDeviceButton->started){
			pdlDeviceButton->toggle();
		}

		pdlScanner = new PdlScanner("PDL Scanner", this);
		
		connect(pdlScanner, SIGNAL(valueChanged(bool)), this, SLOT(setStatusPDL(bool)));
		connect(pdlDevice, SIGNAL(newValue(double)), pdlScanner, SLOT(updateValue(double)));

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

//devices
//only local devices atm
void MainWindow::togglePdlDevice(bool start)
{
	if(start){
		pdlDevice = new PdlDevice(1000, this);
		// connect(pdlDevice, SIGNAL(newValue(double)), this, SLOT(updatePdlValue(double)));
		
		// connect(pdlDeviceButton, SIGNAL(toggle_device), pdlDevice, SLOT(pdlDevice->deleteLater()));
		// status->setText(QString::number(pdlDevice->current_value()));
	}else{
		delete pdlDevice;
		// status->setText(ready_message);
		// return;
	}
}

void MainWindow::updatePdlValue(double value)
{
	status->setText(QString::number(value));
}

