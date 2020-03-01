#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "protocol.h"
#include "controller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE


class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    Protocol    *m_protocol;
    Controller *m_controller;

private:
    Ui::Widget *ui;

//-------------------------------------
public:
     int smt_run_time;
    QStringList m_compareTypeList;
    QByteArray m_rxArray;

    QSerialPort *m_serialPort;
    QTimer      *m_timer_smt;
    QTimer      *m_timer_comPort;

    QTableWidget *m_tableWidget_config;
    QTableWidget *m_tableWidget_home;

    void init();

    void updateConfigMap();

    void updateConfigPage();

    void setHomePage();
    void updateHomePage();

private slots:
    void on_pushButton_open_clicked();
    void on_pushButton_smt_clicked();
    void on_toolButton_config_clicked();

public slots:
    void slot_numCheck_stateChanged(int state);
    void slot_autoStop_stateChanged(int state);//no need
    void slot_IMEI_changed(QString text);
    void slot_comPort_timeout();

    void slot_smt_timeout();
    void slot_readyRead();   

signals:


};


#endif // WIDGET_H
