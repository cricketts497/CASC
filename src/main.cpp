#include <QApplication>

#include "include/MainWindow.h"


int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	MainWindow mainWind;

	//gestures for the graph panning
	mainWind.grabGesture(Qt::PanGesture);

	mainWind.show();
	return app.exec();
}