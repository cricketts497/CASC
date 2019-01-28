#include "include/PdlDevice.h"
#include <QtWidgets>
#include <QFile>

PdlDevice::PdlDevice(uint interval, const QString file_path, QMainWindow *parent) :
QTimer(parent),
value(10),
timestamp(0)
{
	timestamp_interval = interval*2000000;

	fake_pdl_temp_file = new QFile(file_path);
	if(!fake_pdl_temp_file->open(QIODevice::WriteOnly)){
		qDebug() << "Couldn't open new tagger file";
		return;
	}
	QDataStream out(fake_pdl_temp_file);
	out << QString("PDL");
	fake_pdl_temp_file->close();

	connect(this, SIGNAL(timeout()), this, SLOT(increaseValue()));

	start(interval);
}

uint PdlDevice::current_value()
{
	return value;
}

void PdlDevice::increaseValue()
{
	if(!fake_pdl_temp_file->open(QIODevice::WriteOnly)){
		qDebug() << "Couldn't open new tagger file";
		return;
	}
	QDataStream out(fake_pdl_temp_file);

	out << timestamp;
	out << value;

	fake_pdl_temp_file->close();

	value += 5;
	timestamp += timestamp_interval;
	emit newValue(value);
}