#ifndef PDL_SCANNER_H
#define PDL_SCANNER_H

#include <QGroupBox>

class QGridLayout;
class QPushButton;
class QMainWindow;

class PdlScanner : public QGroupBox
{
	Q_OBJECT

public:
	PdlScanner(const char *name, QMainWindow *parent);

	//control booleans
	bool stopped;
	bool up_direction;

private slots:
	void startStopClicked();
	void directionClicked();

signals:
	void valueChanged(bool changed);

private:
	void createButtons();

	QGridLayout *layout;

	//buttons
	QPushButton *startStopButton;
	QPushButton *directionButton;

	//writing to NI card
	void write();
};

#endif //PDL_SCANNER_H