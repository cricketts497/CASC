#include "include/DummyScanner.h"

DummyScanner::DummyScanner(QWidget * parent) :
CascWidget("Dummy scanner", parent),
startButton(new QPushButton("Start", this)),
stopButton(new QPushButton("Stop", this))
{
    QWidget * widget = new QWidget(this);
    setWidget(widget);
    
    QVBoxLayout * vlayout = new QVBoxLayout;
	widget->setLayout(vlayout);
    
    connect(startButton, SIGNAL(clicked()), this, SLOT(startClicked()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopClicked()));
    
    stopButton->setEnabled(false);
    
    vlayout->addWidget(startButton);
    vlayout->addWidget(stopButton);    
    
    setFixedSize(minimumSizeHint());
}

void DummyScanner::startClicked()
{
    startButton->setEnabled(false);
    stopButton->setEnabled(true);    
    
    emit sendCommand(QString("START"));
}

void DummyScanner::stopClicked()
{
    startButton->setEnabled(true);
    stopButton->setEnabled(false);    
    
    emit sendCommand(QString("STOP"));
}
