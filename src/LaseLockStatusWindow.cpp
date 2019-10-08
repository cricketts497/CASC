#include "include/LaseLockStatusWindow.h"

LaseLockStatusWindow::LaseLockStatusWindow(QStringList laselockStatusWindows, QWidget * parent) : 
CascWidget(QString("LaseLock"),parent)
// onMessages({"Locked", "Searching", "In clip", "Hold"}),
// offMessages({"Not locked", "Search OK", "In OK", "Reg OK"}),
// onStates({1,2,2,2}),
// offStates({0,1,1,1})
{
    QWidget * widget = new QWidget(this);
	setWidget(widget);
    
    QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);
    
    QLabel * regALab = new QLabel("Regulator A", this);
    QLabel * regBLab = new QLabel("Regulator B", this);
    layout->addWidget(regALab,0,0);
    layout->addWidget(regBLab,0,1);

    for(int i=0; i<4; i++){
        QELabel * regALabel = new QELabel(QString("CASC:laselock:%1").arg(laselockStatusWindows.at(2*i)), this);
        QELabel * regBLabel = new QELabel(QString("CASC:laselock:%1").arg(laselockStatusWindows.at(2*i+1)), this);
        
        regALabel->activate();
        regBLabel->activate();
        
        layout->addWidget(regALabel,i+1,0);
        layout->addWidget(regBLabel,i+1,1);
    }
    
    widget->setFixedSize(150,200);
}

// void LaseLockStatusWindow::receiveLaseLockStatus(QString status)
// {
    // QStringList status_list = status.split("_");

    // //check for correct format
    // if(status_list.length() != 10 || status_list.first() != QString("Status") || status_list.at(1) != QString("laselock")){
        // return;
    // }
    
    // //regulator A
    // for(int i=2; i<status_list.length(); i+=2){
        // int readoutN = i/2-1;
        // uint set = status_list.at(i).toUInt();
        // switch(set){
            // case 0:
                // regASet[readoutN]->setText(offMessages.at(readoutN));
                // setReadoutState(regASet[readoutN], offStates.at(readoutN));
                // break;
            // case 1:
                // regASet[readoutN]->setText(onMessages.at(readoutN));
                // setReadoutState(regASet[readoutN], onStates.at(readoutN));
                // break;
        // }
    // }
    
    // //regulator B
    // for(int i=3; i<status_list.length(); i+=2){
        // int readoutN = (i-1)/2-1;
        // uint set = status_list.at(i).toUInt();
        // switch(set){
            // case 0:
                // regBSet[readoutN]->setText(offMessages.at(readoutN));
                // setReadoutState(regBSet[readoutN], offStates.at(readoutN));
                // break;
            // case 1:
                // regBSet[readoutN]->setText(onMessages.at(readoutN));
                // setReadoutState(regBSet[readoutN], onStates.at(readoutN));
                // break;
        // }
    // }
// }


// void LaseLockStatusWindow::setReadoutState(ParamReadout * readout, uint state)
// {
    // switch(state){
        // case 0:
            // readout->setOff();
            // break;
        // case 1:
            // readout->setOK();
            // break;
        // case 2:
            // readout->setFail();
            // break;
    // }    
// }
