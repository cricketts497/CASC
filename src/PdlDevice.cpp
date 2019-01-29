#include "include/PdlDevice.h"
#include <QtWidgets>
#include <QFile>

PdlDevice::PdlDevice(uint interval, const QString file_path, QMainWindow *parent) :
QTimer(parent),
value(10),
timestamp(0)
{
	// timestamp_interval = interval*2000000;
	time = new QDateTime();

	connect(this, SIGNAL(timeout()), this, SLOT(increaseValue()));

	fake_pdl_temp_file = new QFile(file_path);
	if(!fake_pdl_temp_file->open(QIODevice::WriteOnly)){
		qDebug() << "Couldn't open new pdl file";
		return;
	}
	QDataStream out(fake_pdl_temp_file);
	quint64 header = time->currentMSecsSinceEpoch();
	out << header;
	fake_pdl_temp_file->close();

	start(interval);
}

uint PdlDevice::current_value()
{
	return value;
}

void PdlDevice::increaseValue()
{
	value += 1;
	timestamp = time->currentMSecsSinceEpoch();

	if(!fake_pdl_temp_file->open(QIODevice::Append)){
		qDebug() << "Couldn't open pdl file";
		return;
	}
	QDataStream out(fake_pdl_temp_file);

	out << timestamp;
	out << value;

	fake_pdl_temp_file->close();

	emit newValue(value);
}