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
    QTimer      *m_smt_timer;    // smt running time
    int         smt_run_time;
    QTimer      *m_comPort_timer;// comPort update
    //QTimer      *m_tryComm_timer;//when smt start,communiation may fail,so need try some times

#define CONFIG_COL_CNT 3
#define HOME_COL_CNT   3
#define CONFIG_WIDGET_INDEX 2
#define HOME_WIDGET_INDEX   1
    QTableWidget *m_tableWidget_home;
    QTableWidget *m_tableWidget_config;

    QByteArray m_rxArray;

    SMTTool *m_smtTool;

    //----------------
    //read yaml
    void loadConfig();
    void saveConfig();

    void dispConfigPage();
    void dispHomePage();

#define MODE_REGULAR 0
#define MODE_TABLE   1
#define GPIO_DISPLAY_MODE MODE_REGULAR
    void setGpioMap(string gpios);
    void dispGpioTable(int row, int col, QString gpios);//MODE_TABLE
    void dispGpioTable(string pin_name, item_t item, int pin_row); //MODE_REGULAR

    QStringList m_compareTypeList;
    bool valueCompare(QString value, QString set_value, QString type);// return result(pass or fail)
private slots:
    void on_pushButton_open_clicked();

    void on_pushButton_smt_clicked();

    void on_toolButton_config_clicked();

public slots:
    void slot_numCheck_stateChanged(int state);
    void slot_autoStop_stateChanged(int state);//no need
    void slot_IMEI_changed(QString text);
    void slot_comPort_timeout();
    //void slot_tryComm_timeout();
    void slot_smt_timeout();
    void slot_readyRead();
    void slot_dispUpdate();
    void slot_dispUpdate_io();
    void slot_sendData(unsigned char *data, int len);//

    void slot_saveLog();

signals:
    void sig_dispUpdate();
    void sig_dispUpdate_io();

private:
    Ui::Widget *ui;
//============================
public:
//#define SELF_TEST_ENABLE
//just for test
#ifdef SELF_TEST_ENABLE
    QTimer *m_test_timer;
    void    test_init();

public slots:
    void slot_test_timeout();
#endif
//============================
};
#endif // WIDGET_H
