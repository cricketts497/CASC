#include "include/ParamReadout.h"

ParamReadout::ParamReadout(const char * text, QWidget * parent) : 
QLineEdit(text, parent)
{
    setReadOnly(true);
    setOff();
}

void ParamReadout::setOff()
{
    QPalette pal = palette();
    pal.setColor(QPalette::Base, off_colour);
    setPalette(pal);
    update();
}

void ParamReadout::setOK()
{
    QPalette pal = palette();
    pal.setColor(QPalette::Base, ok_colour);
    setPalette(pal);
    update();
}

void ParamReadout::setFail()
{
    QPalette pal = palette();
    pal.setColor(QPalette::Base, fail_colour);
    setPalette(pal);
    update();
}