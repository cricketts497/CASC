#include <QtWidgets>
#include "include/PdlScanner.h"

PdlScanner::PdlScanner(const char *name="PDL scanner", QMainWindow *parent=nullptr) :
// PdlScanner::PdlScanner(const char *name="PDL scanner", QDockWidget *parent=nullptr) :
	// QGroupBox(name, parent),
	QDockWidget(name, parent),
	stopped(true),
	up_direction(true)
{
	QWidget *widget = new QWidget(this);
	layout = new QGridLayout;
	widget->setLayout(layout);
	setWidget(widget);

	createButtons();
}

void PdlScanner::closeEvent(QCloseEvent *event)
{
	emit closing(true);
}

void PdlScanner::createButtons()
{
	pdlValue = new QLabel("0");
	layout->addWidget(pdlValue, 0,0,1,2);

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
	layout->addLayout(speedLayout, 1,1);

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
	layout->addLayout(multiplierLayout, 1,0);

	// directionButton = new QPushButton("Down");
	// connect(directionButton, &QAbstractButton::clicked, this, &PdlScanner::directionClicked);
	// layout->addWidget(directionButton, 2,0);
	directionLayout = new QVBoxLayout;
	QButtonGroup *directionGroup = new QButtonGroup;
	QRadioButton *upButton = new QRadioButton("Up");
	directionLayout->addWidget(upButton);
	directionGroup->addButton(upButton, 0);
	QRadioButton *downButton = new QRadioButton("Down");
	directionLayout->addWidget(downButton);
	directionGroup->addButton(downButton, 1);
	connect(directionGroup, SIGNAL(buttonClicked(int)), this, SLOT(directionClicked(int)));
	upButton->setChecked(true);
	layout->addLayout(directionLayout, 2,0);

	startStopButton = new QPushButton("Start");
	connect(startStopButton, &QAbstractButton::clicked, this, &PdlScanner::startStopClicked);
	layout->addWidget(startStopButton, 2,1);

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

void PdlScanner::directionClicked(int id)
{
	// if(up_direction){
	// 	//change to down
	// 	up_direction = false;
	// 	write();
	// 	directionButton->setText("Up");
	// }else{
	// 	//change to up
	// 	up_direction = true;
	// 	write();
	// 	directionButton->setText("Down");
	// }
	currentDirection = id;
	write();
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

void PdlScanner::updateValue(double value)
{
	pdlValue->setText(QString::number(value));
}