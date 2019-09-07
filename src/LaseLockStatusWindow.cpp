#include "include/LaseLockStatusWindow.h"

LaseLockStatusWindow::LaseLockStatusWindow(QWidget * parent) : 
CascWidget(QString("TEM LaseLock"),parent),
onMessages({"Locked", "Searching", "Clipped", "Hold"}),
offMessages({"Not locked", "Search OK", "Out OK", "Reg OK"}),
onStates({1,2,2,2}),
offStates({0,1,1,1})
{
    QWidget * widget = new QWidget(this);
	setWidget(widget);
    
    QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);
    
    QLabel * regALabel = new QLabel("Regulator A", this);
    QLabel * regBLabel = new QLabel("Regulator B", this);
    layout->addWidget(regALabel,0,0);
    layout->addWidget(regBLabel,0,1);
    
    for(int i=0; i<4; i++){
        regASet[i] = new ParamReadout("????", this);
        regBSet[i] = new ParamReadout("????", this);
        
        layout->addWidget(regASet[i],i+1,0);
        layout->addWidget(regBSet[i],i+1,1);
    }
    
    widget->setFixedSize(widget->minimumSizeHint());
}

void LaseLockStatusWindow::receiveLaseLockStatus(QString status)
{
    QStringList status_list = status.split("_");

    //check for correct format
    if(status_list.length() != 10 || status_list.first() != QString("Status") || status_list.at(1) != QString("laselock")){
        return;
    }
    
    //regulator A
    for(int i=2; i<status_list.length(); i+=2){
        int readoutN = i/2-1;
        uint set = status_list.at(i).toUInt();
        switch(set){
            case 0:
                regASet[readoutN]->setText(offMessages.at(readoutN));
                setReadoutState(regASet[readoutN], offStates.at(readoutN));
                break;
            case 1:
                regASet[readoutN]->setText(onMessages.at(readoutN));
                setReadoutState(regASet[readoutN], onStates.at(readoutN));
                break;
        }
    }
    
    //regulator B
    for(int i=3; i<status_list.length(); i+=2){
        int readoutN = (i-1)/2-1;
        uint set = status_list.at(i).toUInt();
        switch(set){
            case 0:
                regBSet[readoutN]->setText(offMessages.at(readoutN));
                setReadoutState(regBSet[readoutN], offStates.at(readoutN));
                break;
            case 1:
                regBSet[readoutN]->setText(onMessages.at(readoutN));
                setReadoutState(regBSet[readoutN], onStates.at(readoutN));
                break;
        }
    }
}


void LaseLockStatusWindow::setReadoutState(ParamReadout * readout, uint state)
{
    switch(state){
        case 0:
            readout->setOff();
            break;
        case 1:
            readout->setOK();
            break;
        case 2:
            readout->setFail();
            break;
    }    
}
