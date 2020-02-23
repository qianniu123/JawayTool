#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "smttool.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void init(void);

    QSerialPort *m_serialPort;
    QTimer      *m_smt_timer;    // smt testing time
    QTimer      *m_comPort_timer;// comPort update
    QTimer      *m_tryComm_timer;

    QTableWidget *m_tableWidget_home;
    QTableWidget *m_tableWidget_config;

    QByteArray m_rxArray;

    SMTTool *m_smtTool;

private slots:
    void on_pushButton_open_clicked();

    void on_pushButton_smt_clicked();

    void on_toolButton_config_clicked();


public slots:
    void slot_numCheck_stateChanged(int state);
    void slot_comPort_timeout();
    void slot_tryComm_timeout();
    void slot_readyRead();

    void slot_sendData(unsigned char *data, int len);//
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
