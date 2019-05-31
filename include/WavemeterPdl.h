#ifndef WAVEMETER_PDL
#define WAVEMETER_PDL

#include "include/LocalDevice.h"

// #include "include/wlmData.h"
#include <QProcess>

class WavemeterPdl : public LocalDevice
{
    Q_OBJECT
public:
    WavemeterPdl(CascConfig * config, QObject * parent = nullptr);

private slots:
    void test_process_output();

private:
    QProcess * test_process;
};

#endif // WAVEMETER_PDL