#ifndef PDL_DEVICE
#define PDL_DEVICE

#include <QTimer>

class QMainWindow;
class QFile;

class PdlDevice : public QTimer
{
	Q_OBJECT	

public:
	PdlDevice(uint interval, const QString file_path, QMainWindow *parent);

	uint current_value();

signals:
	void newValue(int value);

private slots:
	void increaseValue();

private:
	void changeSignStep();

	uint timestamp_interval;
	QFile *fake_pdl_temp_file;

	QDateTime *time;

	quint64 timestamp;
	quint64 value;
	int value_step;
	int steps;
};

#endif //PDL_DEVICE