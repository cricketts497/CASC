#ifndef DEVICE_BUTTON
#define DEVICE_BUTTON

#include <QPushButton>

class QToolBar;

class DeviceButton : public QPushButton
{
	Q_OBJECT

public:
	DeviceButton(const char *name, QToolBar *parent, const char *startTip, const char *stopTip);

	void toggle();

	bool started;

signals:
	void toggle_device(bool start);

private:
	void setButtonColour(QColor colour);

	//button colours
	QColor closed_colour = QColor(Qt::white);
	QColor ok_colour = QColor(Qt::green);
	QColor fail_colour = QColor(Qt::red);

	const char *startTip;
	const char *stopTip;

	bool local;
};

#endif //DEVICE_BUTTON