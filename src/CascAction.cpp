#include "include/CascAction.h"

CascAction::CascAction(const char * widgetIconPath, const char * widgetName, const char * widgetOpenMessage, const char * widgetCloseMessage, QObject * parent) :
QAction(widgetName, parent),
widgetOpenMessage(widgetOpenMessage),
widgetCloseMessage(widgetCloseMessage),
widgetOpen(false)
{
    //icon
    const QIcon actionIcon = QIcon(widgetIconPath);
    setIcon(actionIcon);
    
    //show the message inviting the user to open the widget
    setStatusTip(widgetOpenMessage);
}

//returns whether the widget was open or closed before the toggle
bool CascAction::widgetToggle()
{
    if(widgetOpen){
        setStatusTip(widgetOpenMessage);
        widgetOpen = false;
        return true;
    }else{
        setStatusTip(widgetCloseMessage);
        widgetOpen = true;
        return false;
    }    
}

bool CascAction::widgetIsOpen()
{
    return widgetOpen;
}

void CascAction::widgetSendingCommand(QString command)
{
    emit newWidgetCommand(command);    
}



