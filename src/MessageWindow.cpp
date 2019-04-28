#include "include/MessageWindow.h"
#include <QtWidgets>

MessageWindow::MessageWindow(QWidget * parent) :
CascWidget(QString("Messages"), parent)
{
	// textEdit = new QPlainTextEdit(this);
	textEdit = new QPlainTextEdit(this);
	setWidget(textEdit);
	// textEdit->setReadOnly(true);
	textEdit->appendPlainText(QString("Hello! This is the CASC message log."));
}

void MessageWindow::addMessage(QString message)
{
	textEdit->appendPlainText(message);
}