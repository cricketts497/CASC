#include "include/NxdsPumpStatusWindow.h"

NxdsPumpStatusWindow::NxdsPumpStatusWindow(QStringList pump_names, QWidget * parent) :
CascWidget(QString("nXDS pumps"), parent),
pump_names(pump_names),
nominal_speed(30)
{
    QWidget * widget = new QWidget(this);
	setWidget(widget);
    
    QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);
    
    QLabel * pumpLabel = new QLabel("Pump", this);
    QLabel * rotationLabel = new QLabel("Speed / Hz", this);
    QLabel * temperatureLabel = new QLabel("Temp / C", this);
    QLabel * statusLabel = new QLabel("Status", this);
    
    layout->addWidget(pumpLabel,0,0);
    layout->addWidget(rotationLabel,1,0);
    layout->addWidget(temperatureLabel,2,0);
    layout->addWidget(statusLabel,3,0);
    
    QLabel * pump_name_labels[nPumps];
    for(int i=0; i<nPumps; i++){
        pump_name_labels[i] = new QLabel(pump_names.at(i), this);
        speeds[i] = new ParamReadout("????", this);
        temperatures[i] = new ParamReadout("????", this);
        statuses[i] = new ParamReadout("????", this);
    
        layout->addWidget(pump_name_labels[i],0,i+1);
        layout->addWidget(speeds[i],1,i+1);
        layout->addWidget(temperatures[i],2,i+1);
        layout->addWidget(statuses[i],3,i+1);
    }
    
    widget->setFixedSize(widget->minimumSizeHint());
    
}

void NxdsPumpStatusWindow::receiveNxdsStatus(QString status)
{
    QStringList status_list = status.split("_");
    
    //check for correct format
    if(status_list.length() != 4 || status_list.first() != QString("Status"))
        return;
    
    //find the pump this status is for
    int pump_index=-1;
    for(int i=0; i<nPumps; i++){
        if(status_list.at(1) == pump_names.at(i)){
            pump_index = i;
            break;
        }
    }
    if(pump_index < 0){
        return;
    }
    
    //pump speed status, see NxdsPump.h or nXDS serial comms manual for details
    QStringList speed_status_list = status_list.at(2).split(";");
    
    speeds[pump_index]->setText(speed_status_list[0]);
    int rot_speed = speed_status_list[0].toInt();
    if(rot_speed == nominal_speed){
        speeds[pump_index]->setOK();
    }else{
        speeds[pump_index]->setOff();
    }
    
    int register1 = speed_status_list[1].toInt(nullptr, 16);
    int register2 = speed_status_list[2].toInt(nullptr, 16);
    int warning_register = speed_status_list[3].toInt(nullptr, 16);
    int fault_register = speed_status_list[4].toInt(nullptr, 16);
    
    //pump controller temperature
    temperatures[pump_index]->setText(status_list.at(3));
    temperatures[pump_index]->setOff();
    
    //second register messages
    //alarm, see fault register
    if((register2&0x0080)==0x0080 || fault_register != 0){
        statuses[pump_index]->setText("FAULT");
        statuses[pump_index]->setFail();
        
        //over temperature trip
        if((fault_register&0x0008)==0x0008){
            temperatures[pump_index]->setText("HIGH");
            temperatures[pump_index]->setFail();
        //under temperature trip
        }else if((fault_register&0x0010)==0x0010){
            temperatures[pump_index]->setText("LOW");
            temperatures[pump_index]->setFail();
        }            
    //warning, see warning register
    }else if((register2&0x0040)==0x0040 || warning_register != 0){
        statuses[pump_index]->setText("WARNING");
        statuses[pump_index]->setFail();
        
        //low pump controller temperature
        if((warning_register&0x0002)==0x0002){
            temperatures[pump_index]->setText("LOW");
            temperatures[pump_index]->setFail();
        //high pump controller temperature
        }else if((warning_register&0x0400)==0x0400){
            temperatures[pump_index]->setText("HIGH");
            temperatures[pump_index]->setFail();
        }
    //first register messages
    //decel
    }else if((register1&0x0001)==0x0001){
        statuses[pump_index]->setText("Decelerating");
        statuses[pump_index]->setFail();
    //Standby
    }else if((register1&0x0004)==0x0004){
        statuses[pump_index]->setText("Standby");
        statuses[pump_index]->setFail();
    //accel or running
    }else if((register1&0x0002)==0x0002){
        statuses[pump_index]->setText("Running");
        statuses[pump_index]->setOK();
    //if none of these are given, probably that the pump is off
    }else{
        statuses[pump_index]->setText("Pump off");
        statuses[pump_index]->setFail();
    }      
    
    
}