#include <QApplication>

#include "include/MainWindow.h"


int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	MainWindow mainWind;
	mainWind.show();
	return app.exec();
}