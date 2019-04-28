#include "include/DummyScanner.h"

DummyScanner::DummyScanner(QWidget * parent) :
CascWidget(QString("Dummy scanner"), parent),
startButton(new QPushButton("Start", this)),
stopButton(new QPushButton("Stop", this))
{
    QWidget * widget = new QWidget(this);
    setWidget(widget);
    
    QVBoxLayout * vlayout = new QVBoxLayout;
	widget->setLayout(vlayout);
    
    connect(startButton, SIGNAL(clicked()), this, SLOT(startClicked()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopClicked()));
    
    startButton->setEnabled(false);
    stopButton->setEnabled(false);
    
    vlayout->addWidget(startButton);
    vlayout->addWidget(stopButton);    
    
    setFixedSize(minimumSizeHint());
}

void DummyScanner::dataSaverOn(bool on)
{
    if(on && !stopButton->isEnabled()){
        startButton->setEnabled(true);
    }else if(!on){
        startButton->setEnabled(false);
        stopButton->setEnabled(false);
    }        
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
