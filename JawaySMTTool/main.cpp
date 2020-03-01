#include "widget.h"
#include "protocol.h"
#include "control.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Protocol *proto = new Protocol();
    Controller *controller = new Controller();
    controller->m_proto = proto;

    Widget w;
    w.m_protocol = proto;
    w.m_controller = controller;

    w.show();

    //emit ui->toolButton_config->clicked();

    return a.exec();
}
