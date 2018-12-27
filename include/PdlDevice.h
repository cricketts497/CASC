#ifndef PDL_DEVICE
#define PDL_DEVICE

#include <QTimer>

class QMainWindow;

class PdlDevice : public QTimer
{
	Q_OBJECT	

public:
	PdlDevice(int interval, QMainWindow *parent);

	double current_value();

signals:
	void newValue(double value);

private slots:
	void increaseValue();

private:
	double value;

};

#endif //PDL_DEVICE