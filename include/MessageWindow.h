#ifndef MESSAGE_WINDOW
#define MESSAGE_WINDOW

#include <QDockWidget>

class QPlainTextEdit;

class MessageWindow : public QDockWidget
{
	Q_OBJECT
public:
	MessageWindow(QWidget * parent=nullptr);

protected:
	void closeEvent(QCloseEvent *event);

signals:
	void closing();

public slots:
	void addMessage(QString message);

private:
	QPlainTextEdit * textEdit;

};


#endif //MESSGAE_WINDOW