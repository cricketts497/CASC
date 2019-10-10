#include "include/AgilentTV301StatusWindow.h"

AgilentTV301StatusWindow::AgilentTV301StatusWindow(QStringList pump_names, QWidget * parent) :
CascWidget(QString("Agilent Turbos"), parent)
{
    QWidget * widget = new QWidget(this);
	setWidget(widget);
    
    QGridLayout * layout = new QGridLayout;
	widget->setLayout(layout);
    
    QLabel * pumpLabel = new QLabel("Pump", this);
    QLabel * driveLabel = new QLabel("Drive / Hz", this);
    QLabel * temperatureLabel = new QLabel("Temp / C", this);
    QLabel * statusLabel = new QLabel("Status", this);
    QLabel * errorLabel = new QLabel("Error", this);

    layout->addWidget(pumpLabel,0,0);
    layout->addWidget(driveLabel,1,0);
    layout->addWidget(temperatureLabel,2,0);
    layout->addWidget(statusLabel,3,0);
    layout->addWidget(errorLabel,4,0);
    
    // if(pump_names.length() != nAgilentTV301Pumps){
        // emit widget_message("AGILENTTV301STATUSWINDOW ERROR: Incorrect number of pumps in pump_names");
        // return;
    // }
    
    // QLabel * pump_name_labels[nAgilentTV301Pumps];
    // for(int i=0; i<nAgilentTV301Pumps; i++){
        // pump_name_labels[i] = new QLabel(pump_names.at(i), this);
        // drives[i] = new ParamReadout("????", this);
        // temperatures[i] = new ParamReadout("????", this);
        // statuses[i] = new ParamReadout("????", this);
    
        // layout->addWidget(pump_name_labels[i],0,i+1);
        // layout->addWidget(drives[i],1,i+1);
        // layout->addWidget(temperatures[i],2,i+1);
        // layout->addWidget(statuses[i],3,i+1);
    // }
    
    for(int i=0; i<pump_names.length(); i++){
        QLabel * pump_name_label = new QLabel(pump_names.at(i), this);
        QELabel * drive = new QELabel(QString("CASC:Turbo:%1:Drive").arg(pump_names.at(i).mid(5)));
        QELabel * temperature = new QELabel(QString("CASC:Turbo:%1:Temperature").arg(pump_names.at(i).mid(5)));
        QELabel * status = new QELabel(QString("CASC:Turbo:%1:Status").arg(pump_names.at(i).mid(5)));
        QELabel * error = new QELabel(QString("CASC:Turbo:%1:Error").arg(pump_names.at(i).mid(5)));
        
        drive->activate();
        temperature->activate();
        status->activate();
        error->activate();
        
        layout->addWidget(pump_name_label,0,i+1);
        layout->addWidget(drive,1,i+1);
        layout->addWidget(temperature,2,i+1);
        layout->addWidget(status,3,i+1);
        layout->addWidget(error,4,i+1);
    }
    
    widget->setFixedSize(430,250);
}


// void AgilentTV301StatusWindow::receiveAgilentTV301Status(QString status)
// {
    // QStringList status_list = status.split("_");

    // //check for correct format
    // if(status_list.length() != 6 || status_list.first() != QString("Status"))
        // return;
    
    // //find the pump this status is for
    // int pump_index=-1;
    // for(int i=0; i<nAgilentTV301Pumps; i++){
        // if(status_list.at(1) == pump_names.at(i)){
            // pump_index = i;
            // break;
        // }
    // }
    // if(pump_index < 0){
        // return;
    // }
    
    // //status code
    // int statusCode = status_list.at(2).toInt();
    // switch(statusCode){
        // case 0:
            // statuses[pump_index]->setText("Stopped");
            // statuses[pump_index]->setFail();
            // break;
        // case 1:
            // statuses[pump_index]->setText("Interlock");
            // statuses[pump_index]->setFail();
            // break;
        // case 2:
            // statuses[pump_index]->setText("Starting");
            // statuses[pump_index]->setOff();
            // break;
        // case 3:
            // statuses[pump_index]->setText("Tuning");
            // statuses[pump_index]->setOff();
            // break;
        // case 4:
            // statuses[pump_index]->setText("Braking");
            // statuses[pump_index]->setFail();
            // break;
        // case 5:
            // statuses[pump_index]->setText("Running");
            // statuses[pump_index]->setOK();
            // break;
        // case 6:
            // statuses[pump_index]->setText("FAULT");
            // statuses[pump_index]->setFail();
            // break;
    // }
    
    // //temperature
    // temperatures[pump_index]->setText(status_list.at(4));
    // temperatures[pump_index]->setOff();
    
    // //drive
    // drives[pump_index]->setText(status_list.at(5));
    // int drive = status_list.at(5).toUInt();
    // if(drive == nominal_speed){
        // drives[pump_index]->setOK();
    // }else{
        // drives[pump_index]->setOff();
    // }
    
    // //error code
    // uint errorCode = status_list.at(3).toUInt();
    // if((errorCode&0x0002)==0x0002){
        // temperatures[pump_index]->setText("HIGH");
        // temperatures[pump_index]->setFail();
    // }
    // if(errorCode != 0){
        // statuses[pump_index]->setText("FAULT");
        // statuses[pump_index]->setFail();
    // }
// }





