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
	speedLayout = new QVBoxLayout;
	QRadioButton *speedButtons[nSpeedButtons];
	QButtonGroup *speedGroup = new QButtonGroup;
	QLabel *speedLabel = new QLabel(speed_name);
	speedLabel->setAlignment(Qt::AlignHCenter);
	speedLayout->addWidget(speedLabel);
	for(int i=0; i<nSpeedButtons; i++){
		speedButtons[i] = new QRadioButton(QString::number(speed_values[i]));
		speedLayout->addWidget(speedButtons[i]);
		speedGroup->addButton(speedButtons[i], i);
	}
	connect(speedGroup, SIGNAL(buttonClicked(int)), this, SLOT(speedClicked(int)));
	speedButtons[speedDefault]->setChecked(true);
	// speedLayout->setSpacing(2);
	layout->addLayout(speedLayout, 0,0);

	multiplierLayout = new QVBoxLayout;
	QRadioButton *multiplierButtons[nMultiplierButtons];
	QButtonGroup *multiplierGroup = new QButtonGroup;
	QLabel *multiplierLabel = new QLabel(multiplier_name);
	multiplierLabel->setAlignment(Qt::AlignHCenter);
	multiplierLayout->addWidget(multiplierLabel);
	for(int i=0; i<nMultiplierButtons; i++){
		multiplierButtons[i] = new QRadioButton(QString::number(multiplier_values[i]));
		multiplierLayout->addWidget(multiplierButtons[i]);
		multiplierGroup->addButton(multiplierButtons[i], i);
	}
	connect(multiplierGroup, SIGNAL(buttonClicked(int)), this, SLOT(multiplierClicked(int)));
	multiplierButtons[multiplierDefault]->setChecked(true);
	layout->addLayout(multiplierLayout, 0,1);

	startStopButton = new QPushButton("Start");
	connect(startStopButton, &QAbstractButton::clicked, this, &PdlScanner::startStopClicked);
	layout->addWidget(startStopButton, 1,1);

	directionButton = new QPushButton("Down");
	connect(directionButton, &QAbstractButton::clicked, this, &PdlScanner::directionClicked);
	layout->addWidget(directionButton, 1,0);

	setFixedSize(layout->minimumSize());
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
		//change to down
		up_direction = false;
		write();
		directionButton->setText("Up");
	}else{
		//change to up
		up_direction = true;
		write();
		directionButton->setText("Down");
	}
}

void PdlScanner::speedClicked(int id)
{
	currentSpeed = id;
	write();
}

void PdlScanner::multiplierClicked(int id)
{
	currentMultiplier = id;
	write();
}

void PdlScanner::write()
{
	emit valueChanged(true);
	return;
}
