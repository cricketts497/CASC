#ifndef CASC_ACTION
#define CASC_ACTION

#include <QAction>

class CascAction : public QAction
{
    Q_OBJECT
public:
    CascAction(const char * iconPath, const char * widgetName, const char * widgetOpenMessage, const char * widgetCloseMessage, QObject * parent = nullptr);
    
public slots:
    void widgetSendingCommand(QString command);
    bool widgetToggle();
    
    bool widgetIsOpen();
    
signals:
    void newWidgetCommand(QString command);

private:
    const char * widgetOpenMessage;
    const char * widgetCloseMessage;
    
    bool widgetOpen;
};

#endif // CASC_ACTION