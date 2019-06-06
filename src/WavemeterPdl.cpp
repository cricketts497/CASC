#include "include/WavemeterPdl.h"

WavemeterPdl::WavemeterPdl(CascConfig * config, QObject * parent) :
LocalDevice(QString("wavemeterpdl"), config, parent),
test_process(new QProcess(this))
{
    if(device_failed)
        return;
    
    //arguments: 0=>wavelngth meter type, 1=>version number, 2=>software revision number, 3=>software compilation number
    // int version_number = GetWLMVersion(0);
    connect(test_process, SIGNAL(readyReadStandardOutput()), this, SLOT(test_process_output()));
    test_process->start("./dependencies/test.py");
}

void WavemeterPdl::test_process_output()
{    
    QString output = QString(test_process->readAllStandardOutput());
    
    storeMessage(QString("Local wavemeterpdl: Test output: %1").arg(output), false);
    emit device_message(QString("Local wavemeterpdl: Test output: %1").arg(output));
}