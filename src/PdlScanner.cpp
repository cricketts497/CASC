#include <QtWidgets>
#include "include/PdlScanner.h"

PdlScanner::PdlScanner(const char *name="PDL scanner", QMainWindow *parent=nullptr) :
	QGroupBox(name, parent),
	stopped(true),
	up_direction(true)
{
	layout = new QGridLayout;
	setLayout(layout);

	createButtons();
}

void PdlScanner::createButtons()
{
	startStopButton = new QPushButton("Start");
	connect(startStopButton, &QAbstractButton::clicked, this, &PdlScanner::startStopClicked);
	layout->addWidget(startStopButton, 0,1);

	directionButton = new QPushButton("Down");
	connect(directionButton, &QAbstractButton::clicked, this, &PdlScanner::directionClicked);
	layout->addWidget(directionButton, 0,0);
}


//slots
void PdlScanner::startStopClicked()
{
	if(stopped){
		//start the scan
		stopped = false;
		write();
		startStopButton->setText("Stop");
	}else{
		//stop the scan
		stopped = true;
		write();
		startStopButton->setText("Start");
	}
}

void PdlScanner::directionClicked()
{
	if(up_direction){
		up_direction = false;
		write();
		directionButton->setText("Up");
	}else{
		up_direction = true;
		write();
		directionButton->setText("Down");
	}
}

void PdlScanner::write()
{
	emit valueChanged(true);
	return;
}
