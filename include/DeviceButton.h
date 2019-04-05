#ifndef DEVICE_BUTTON
#define DEVICE_BUTTON

#include <QPushButton>

class QToolBar;

class DeviceButton : public QPushButton
{
	Q_OBJECT

public:
	DeviceButton(const char *name, QWidget *parent=nullptr, const char *startTip="Start", const char *stopTip="Stop", const char *failTip="FAIL");

	void toggle();

	bool started;

signals:
	void button_message(QString message);

	void toggle_device(bool start);

public slots:
	void setFail();

private:
	// void setButtonColour(QColor colour);

	//button colours
	const QColor closed_colour = QColor(Qt::white);
	const QColor ok_colour = QColor(Qt::green);
	const QColor fail_colour = QColor(Qt::red);

	const char *startTip;
	const char *stopTip;
	const char *failTip;
};

#endif //DEVICE_BUTTON