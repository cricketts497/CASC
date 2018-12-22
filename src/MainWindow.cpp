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

	pdlDeviceButton = new QPushButton("PDL");

	//set grey
	setButtonColour(pdlDeviceButton, QColor(Qt::white));
	pdlDeviceButton->setAutoFillBackground(true);

	pdlDeviceButton->setStatusTip("Start the PDL scanner device");
	connect(pdlDeviceButton, &QAbstractButton::clicked, this, &MainWindow::toggleDevicePdl);
	devicesBar->addWidget(pdlDeviceButton);

	addToolBar(Qt::LeftToolBarArea, devicesBar);
}

void MainWindow::toggleDevicePdl()
{
	if(device_PDL_started){
		pdlDeviceButton->setFlat(false);
		setButtonColour(pdlDeviceButton, QColor(Qt::white));
		device_PDL_started = false;
		statusBar()->showMessage("PDL Device Stopped", 2000);
		pdlDeviceButton->setStatusTip("Start the PDL scanner device");
	} else{
		pdlDeviceButton->setFlat(true);
		setButtonColour(pdlDeviceButton, QColor(Qt::green));
		device_PDL_started = true;
		statusBar()->showMessage("PDL Device Started", 2000);
		pdlDeviceButton->setStatusTip("Stop the PDL scanner device");
	}
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
		if (!device_PDL_started){
			toggleDevicePdl();
		}		

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

void MainWindow::setButtonColour(QPushButton *button, QColor colour)
{
	QPalette pal = button->palette();
	pal.setColor(QPalette::Button, colour);
	button->setPalette(pal);
	button->update();
}