#ifndef PDL_SCANNER_H
#define PDL_SCANNER_H

// #include <QGroupBox>
#include <QDockWidget>

class QGridLayout;
class QVBoxLayout;
class QPushButton;
// class QMainWindow;
class QDockWidget;
class QLabel;

// class PdlScanner : public QGroupBox
class PdlScanner : public QDockWidget
{
	Q_OBJECT

public:
	PdlScanner(const char *name, QMainWindow *parent);
	// PdlScanner(const char *name, QDockWidget *parent);

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void startStopClicked();
	void directionClicked(int id);
	void speedClicked(int id);
	void multiplierClicked(int id);

	void updateValue(double value);

signals:
	void valueChanged(bool x);
	void closing(bool x);

private:
	void createButtons();

	QGridLayout *layout;

	//current PDL value
	QLabel *pdlValue;

	//buttons
	static const int nSpeedButtons = 5;
	QVBoxLayout *speedLayout;
	const char *speed_name = "Speed / nm/s";
	const int speedDefault = 0;

	static const int nMultiplierButtons = 4;
	QVBoxLayout *multiplierLayout;
	const char *multiplier_name = "Multiplier";
	const int multiplierDefault = 0;

	QPushButton *startStopButton;

	//id: 0=>up, 1=>down
	// QPushButton *directionButton;
	QVBoxLayout *directionLayout;
	const int directionDefault = 0;

	//writing to NI card
	void write();

public:
	//control booleans
	bool stopped;
	bool up_direction;

	const double speed_values[nSpeedButtons] = {10.0, 1.0, 0.1, 0.01, 0.001};
	int currentSpeed = speedDefault;

	const double multiplier_values[nMultiplierButtons] = {1, 0.5, 0.25, 0.05};
	int currentMultiplier = multiplierDefault;

	int currentDirection = directionDefault;
};

#endif //PDL_SCANNER_H