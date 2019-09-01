#ifndef DEVICE_BUTTON
#define DEVICE_BUTTON

#include <QPushButton>

class QToolBar;

class DeviceButton : public QPushButton
{
	Q_OBJECT

public:
	DeviceButton(const char *name, QWidget *parent=nullptr, const char *startTip="Start", const char *stopTip="Stop", const char *failTip="FAIL");

    bool deviceIsRunning();
	bool deviceToggle();
    
signals:
    void newDeviceStatus(QString status);
    void stop_device();

public slots:
    void device_status(QString status);
	void setFail();
    void deviceHasStopped();

private:
    bool started;

	//button colours
	const QColor closed_colour = QColor(Qt::white);
	const QColor ok_colour = QColor(Qt::green);
	const QColor fail_colour = QColor(Qt::red);

	const char *startTip;
	const char *stopTip;
	const char *failTip;
};

#endif //DEVICE_BUTTON