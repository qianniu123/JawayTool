#include "widget.h"
#include "ui_widget.h"
#include <QCheckBox>

#include <QStackedWidget>
#include <QTableView>
#include <QTableWidgetItem>
#include <QLayout>
#include <QHeaderView>
#include <QFile>
#include <QDateTime>

#include <QDebug>

#include <iostream>
#include <fstream>

#include "yaml-cpp/yaml.h"

using namespace std;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setLayout(ui->verticalLayout_all);
    this->setWindowTitle(QString("嘉为SMT测试工具_V3.0"));

    //--------------------------------------------------------------------------------------------------
    ui->pushButton_smt->setEnabled(false);

    connect(ui->checkBox_numCheck, &QCheckBox::stateChanged, this, &Widget::slot_numCheck_stateChanged);
    connect(ui->checkBox_autoStop, &QCheckBox::stateChanged, this, &Widget::slot_autoStop_stateChanged);
    ui->checkBox_autoStop->setCheckState(Qt::Checked);
    ui->checkBox_numCheck->setCheckState(Qt::Unchecked);

    ui->groupBox_numCheck->setLayout(ui->verticalLayout_numCheck);
    ui->groupBox_numCheck->setVisible(false);
    //in horizontalLayout_sn
    ui->label_SN->setVisible(false);
    ui->lineEdit_SN->setVisible(false);

    connect(ui->lineEdit_IMEI, &QLineEdit::textChanged, this, &Widget::slot_IMEI_changed);

    ui->label_result->setAlignment(Qt::AlignCenter);
    QFont result_font;
    result_font.setBold(true);
    result_font.setPixelSize(50);
    ui->label_result->setFont(result_font);

    //------------------------------------------------
    m_tableWidget_home = new QTableWidget();
    m_tableWidget_config = new QTableWidget();
    ui->stackedWidget->addWidget(m_tableWidget_home);
    ui->stackedWidget->addWidget(m_tableWidget_config);

    //------------------------------------
    m_timer_smt = new QTimer();
    m_timer_smt->setInterval(1000);//ms
    connect(m_timer_smt, &QTimer::timeout, this, &Widget::slot_smt_timeout);

    m_timer_comPort = new QTimer();
    m_timer_comPort->setInterval(1000);
    connect(m_timer_comPort, &QTimer::timeout, this, &Widget::slot_comPort_timeout);
    m_timer_comPort->start();

    m_serialPort = new QSerialPort();
    connect(m_serialPort, &QSerialPort::readyRead, this, &Widget::slot_readyRead);


    //----------------------------------------------------------------------


    //-------
    init();

    #ifdef SELF_TEST_ENABLE
        test_init();
    #endif
}

void Widget::init()
{
    m_protocol = nullptr;
    m_controller = nullptr;
    smt_run_time = 0;

    m_compareTypeList << "int" << "string" << "bool" << "flag" << "io";
    //--------------------------------------------------------


    //emit ui->toolButton_config->clicked();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::slot_comPort_timeout()
{
    static QList<QSerialPortInfo> oldComPorts;
    QList<QSerialPortInfo> comPorts = QSerialPortInfo::availablePorts();
    if(oldComPorts.size() != comPorts.size())
    {
        oldComPorts.clear();
        oldComPorts = comPorts;

        //refresh com ports
        QStringList portList;
        foreach (const QSerialPortInfo &portInfo, comPorts)
        {
              portList << portInfo.portName();
        }
        ui->comboBox_comPort->clear();
        ui->comboBox_comPort->addItems(portList);
    }
}



#ifdef SELF_TEST_ENABLE
void Widget::test_init()
{
    m_test_timer = new QTimer();
    m_test_timer->setInterval(2000);
    connect(m_test_timer, &QTimer::timeout, this, &Widget::slot_test_timeout);
    //m_test_timer->start();
}

void Widget::slot_test_timeout()
{
    //set value for test
    if(!m_serialPort->isOpen() || !m_timer_smt->isActive())
    {
        return;
    }

    m_smtTool->frame_map["ADC"] = to_string(125);
    m_smtTool->frame_map["ADC_FLAG"] = "1";
    m_smtTool->frame_map["BLE_MAC"] = "";
    m_smtTool->frame_map["GPS"] = to_string(46);
    m_smtTool->frame_map["GSM"] = to_string(35);
    m_smtTool->frame_map["GSensor"] = "1";
    m_smtTool->frame_map["HARDVER"] = "JWMB1V01";
    m_smtTool->frame_map["ICCID"] = "123456";
    m_smtTool->frame_map["IMEI"] = "";
    m_smtTool->frame_map["LOCK"] = "0";
    m_smtTool->frame_map["外电"] = "1";
    m_smtTool->frame_map["电门"] = "0";

    //m_smtTool->frame_map["GPIO"] = to_string(0x5a5a);


    unsigned char gpio_c[4] = {0xFF, 0xFF, 0xFF, 0x00};
    int *gpio_i = (int*)gpio_c;
    int gpio = *gpio_i;
    int i=0;

    vector<string>::iterator iter;
    for(iter = m_smtTool->gpio_list.begin(); iter != m_smtTool->gpio_list.end(); iter++, i++)
    {
        //bool io_flag = (gpio&0x1) >> i;
        bool io_flag = gpio&(0x1<<i);
        m_smtTool->frame_map[*iter] = to_string(io_flag);
    }

    emit sig_dispUpdate();
}
#endif

void Widget::slot_numCheck_stateChanged(int state)
{   
    //checked=2 , unchecked=0
    ui->groupBox_numCheck->setVisible((state==2)?true:false);
    if(state == 2)//checked
    {
        ui->lineEdit_IMEI->setFocus(); //forcus
    }
    else if(state == 0)
    {
        ui->lineEdit_IMEI->clear();
    }

//    string node_name = ui->label_IMEI->text().toStdString();
//    item_t item = m_smtTool->home_map[node_name];
//    item.value = "";
//    m_smtTool->home_map[node_name] = item;
}

void Widget::slot_autoStop_stateChanged(int state)
{
    qDebug() << "autoStop: " << state;
}

void Widget::slot_IMEI_changed(QString text)
{
    string imei_value = text.toStdString();
    item_t item = m_protocol->home_map["IMEI"];
    item.value = imei_value;
    m_protocol->home_map["IMEI"] = item;

    qDebug() << "home_map new imei = " << text;
}

void Widget::on_pushButton_open_clicked()
{
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);

    m_serialPort->setReadBufferSize(1024);
    //m_serialPort->waitForReadyRead(100);
    m_serialPort->setPortName(ui->comboBox_comPort->currentText());

    if(ui->pushButton_open->text() == QString("打开串口"))
    {
        bool ret = m_serialPort->open(QIODevice::ReadWrite);
        if(ret)
        {
            ui->pushButton_open->setText(QString("关闭串口"));
            ui->pushButton_smt->setEnabled(true);
        }
    }
    else if(ui->pushButton_open->text() == QString("关闭串口"))
    {
        m_serialPort->close();
        ui->pushButton_open->setText(QString("打开串口"));
        ui->pushButton_smt->setEnabled(false);
    }
}

void Widget::on_pushButton_smt_clicked()
{
    if(ui->pushButton_smt->text() == QString("SMT测试"))
    {
        ui->pushButton_smt->setText(QString("停止测试"));

//        m_smtTool->start();//write start data
//        m_smtTool->smt_state = START;//start

        smt_run_time = 0;
        m_timer_smt->start();//smt running
        ui->label_spendTime->setText(QString::number(0));        

        #ifdef SELF_TEST_ENABLE
            m_test_timer->start();
        #endif
    }
    else if(ui->pushButton_smt->text() == QString("停止测试"))
    {
        ui->pushButton_smt->setText(QString("SMT测试"));

//        m_smtTool->stop();//write stop data
//        m_smtTool->smt_state = STOP;//stop
        //m_timer_smt->stop();

        #ifdef SELF_TEST_ENABLE
            m_test_timer->stop();
        #endif

        //save log
        //slot_saveLog();
    }
}

void Widget::slot_smt_timeout()
{
    static int start_timeout = 0;
    static int stop_timeout = 0;
    static QString test_state;
/*
    switch(m_smtTool->smt_state)
    {
        case START:
        {
            m_smtTool->start(); //write start_buf
            smt_run_time++;
            start_timeout++;
            stop_timeout = 0;
            test_state = "start";
        }
        break;
        case STOP:
        {
            m_smtTool->stop(); //write stop_buf
            start_timeout = 0;
            stop_timeout++;
            test_state = "stop";
        }
        break;
        case RUNING://<---SMTTool::msg_process()
        {
            smt_run_time++;
            start_timeout = 0;
            test_state = "testing";
        }
        break;
        case STOPING:
        {
            stop_timeout = 0;
            test_state = "stop";
        }
        break;
    }

    //communicate error timeout
    if(start_timeout >= 10 || stop_timeout >= 5)
    {
        if(m_smtTool->smt_state == START)
        {
            test_state = QString("com error");
            if(ui->checkBox_autoStop->isChecked())
            {
                emit ui->pushButton_smt->clicked(); //auto stop
            }
        }
        else if(m_smtTool->smt_state == STOP)
        {
            m_smtTool->smt_state = STOPING;
        }
    }
*/
    //---------------------------------------------------------
    //smt_run_time++;
    ui->label_spendTime->setText(QString::number(smt_run_time));
    ui->label_result->setText(test_state);
}

void Widget::on_toolButton_config_clicked()
{
    if(ui->toolButton_config->text() == QString("配置"))
    {
        ui->toolButton_config->setText(QString("保存"));

        m_controller->loadConfig();
        updateConfigPage();//config_map --> widget

        ui->stackedWidget->setCurrentWidget(m_tableWidget_config);
    }
    else if(ui->toolButton_config->text() == QString("保存"))
    {
       ui->toolButton_config->setText(QString("配置"));

       updateConfigMap();//widget --> config_map
       m_controller->saveConfig();//config_map --> config.yaml
       m_controller->setHomeMap();//config_map --> home_map
       setHomePage();//home_map --> widget

       ui->stackedWidget->setCurrentWidget(m_tableWidget_home);
    }
}

void Widget::updateConfigPage() //config_map --> config page
{
    //clear table
    m_tableWidget_config->clear();
    QStringList headerList;
    headerList << QString("测试项") << QString("设置值") << QString("类型");
    m_tableWidget_config->setHorizontalHeaderLabels(headerList);
    m_tableWidget_config->setColumnCount(3);

    //create table
    unordered_map<string, item_t> *p_map = &m_protocol->config_map;
    unordered_map<string, item_t>::iterator iter;
    int row_index = 0;
    for(iter=p_map->begin(); iter!=p_map->end(); iter++, row_index++)
    {
        m_tableWidget_config->insertRow(row_index);
        //config_map -->widget
        string node_name = iter->first;
        string enable    = iter->second.enable;
        string set_value = iter->second.value;
        string type      = iter->second.type;
        //qDebug() << "configPage:" << QString::fromStdString(node_name);
        QTableWidgetItem *widgetItem_c0 = new QTableWidgetItem();
        widgetItem_c0->setText(QString::fromStdString(node_name));
        m_tableWidget_config->setItem(row_index, 0, widgetItem_c0);

        QTableWidgetItem *widgetItem_c1 = new QTableWidgetItem();
        widgetItem_c1->setText(QString::fromStdString(set_value));
        m_tableWidget_config->setItem(row_index, 1, widgetItem_c1);

        //QComboBox *widget_c2 = new QComboBox();

    }

}
void Widget::updateConfigMap()//widget --> config_map
{  
    int row_cnt = m_tableWidget_config->rowCount();
    for(int i=0; i<row_cnt; i++)
    {
        QTableWidgetItem *p_item_c0 = m_tableWidget_config->item(i,0);
        QTableWidgetItem *p_item_c1 = m_tableWidget_config->item(i,1);
        //QTableWidgetItem *p_item_c2 = m_tableWidget_config->item(i,2);

        if(p_item_c0 && p_item_c1)
        {
            Qt::CheckState c0_state = p_item_c0->checkState();
            string enable = (c0_state==Qt::Checked?"1":"0");
            string node_name = p_item_c0->text().toStdString();
            string set_value = p_item_c1->text().toStdString();
            string type;

            m_protocol->config_map[node_name] = item_t{enable, set_value, type};
        }
    }
}

void Widget::setHomePage() //home_map --> widget
{
    //clear table
    m_tableWidget_home->clear();
    QStringList headerList;
    headerList << QString("测试项") << QString("值") << QString("结果");
    m_tableWidget_home->setHorizontalHeaderLabels(headerList);
    m_tableWidget_home->setColumnCount(3);

    //create table
    unordered_map<string, item_t> *p_map = &m_protocol->home_map;
    unordered_map<string, item_t>::iterator iter;
    int row_index = 0;
    for(iter=p_map->begin(); iter!=p_map->end(); iter++, row_index++)
    {
        m_tableWidget_home->insertRow(row_index);

        //config_map -->widget
        string node_name = iter->first;
        string enable    = iter->second.enable;
        string set_value = iter->second.value;
        string type      = iter->second.type;
        qDebug() << "homePage:" << QString::fromStdString(node_name);
        QTableWidgetItem *widgetItem_c0 = new QTableWidgetItem();
        widgetItem_c0->setText(QString::fromStdString(node_name));
        m_tableWidget_home->setItem(row_index, 0, widgetItem_c0);

        QTableWidgetItem *widgetItem_c1 = new QTableWidgetItem();
        widgetItem_c1->setText(QString::fromStdString(set_value));
        m_tableWidget_home->setItem(row_index, 1, widgetItem_c1);

        //QComboBox *widget_c2 = new QComboBox();

    }
}

void Widget::updateHomePage() //frame_map --> home page
{
    //c0_text --> frame_map --> value --> c1_text
    int row_cnt = m_tableWidget_home->rowCount();
    for(int i=0; i<row_cnt; i++)
    {
        QTableWidgetItem *p_item_c0 = m_tableWidget_home->item(i, 0);
        QTableWidgetItem *p_item_c1 = m_tableWidget_home->item(i, 1);

        QString c0_text = p_item_c0->text();
        string node_name = c0_text.toStdString();
        string new_value = m_protocol->frame_map[node_name].value;
        p_item_c1->setText(QString::fromStdString(new_value));
    }
}

void Widget::slot_readyRead()
{
    QByteArray rxArray = m_serialPort->readAll();
    //m_serialPort->waitForReadyRead(100);

    static bool frame_head_flag = false;
    if(!rxArray.isEmpty())
    {
        m_rxArray.append(rxArray);
        while(!frame_head_flag && m_rxArray.length()>2)
        {
            if((unsigned char)m_rxArray.at(0) == 0xb6 && (unsigned char)m_rxArray.at(1) == 0x6b)
            {
                //qDebug() << "get frame head; m_rxArray.size = " << m_rxArray.length();
                frame_head_flag = true;
                break;
            }
            else
            {
                frame_head_flag = false;
                m_rxArray.remove(0,1);
                //qDebug() << "remove wrong data";
            }
        }

        if((unsigned)m_rxArray.size() >= 6 && (unsigned)m_rxArray.size() >= ((unsigned)m_rxArray.at(4)+6+2))
        {
            //msg process
            //first: QByteArray --> unsigned char array
            char frame_buff[1024] = {0};
            char *p_frame_buff = frame_buff;
            int frame_len = m_rxArray.at(4)+6+2;

            QByteArray::iterator iter;
            for(iter=m_rxArray.begin(); iter!=m_rxArray.end(); iter++)
            {
                *p_frame_buff = *iter;
                p_frame_buff++;
            }

            //then msg_process
            int result = 0;//m_smtTool->msg_process(frame_buff, frame_len);
            if(result == -1 || result == 1) // -1: data error; 1: data right; 0:part data
            {
                //qDebug() << "-------------get a frame, and clear-------------: result = " << result;
                m_rxArray.clear();
                frame_head_flag = false;
            }

            //emit sig_dispUpdate();
        }
    }
}


