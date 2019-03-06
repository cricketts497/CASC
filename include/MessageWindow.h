#ifndef MESSAGE_WINDOW
#define MESSAGE_WINDOW

#include "include/CascWidget.h"

class MessageWindow : public CascWidget
{
	Q_OBJECT
	
public:
	MessageWindow(QWidget * parent=nullptr);

public slots:
	void addMessage(QString message);

private:
	QPlainTextEdit * textEdit;

};

#endif //MESSGAE_WINDOW