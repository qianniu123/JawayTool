#include "widget.h"
#include "ui_widget.h"
#include <QCheckBox>

#include <QStackedWidget>
#include <QTableView>
#include <QTableWidgetItem>
#include <QLayout>
#include <QHeaderView>

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
    this->setWindowTitle(QString("嘉为SMT测试工具_V2.0"));//JawaySMTTool_V2.0

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

    //------------------------------------
    m_smt_timer = new QTimer();
    m_smt_timer->setInterval(1000);//ms
    connect(m_smt_timer, &QTimer::timeout, this, &Widget::slot_smt_timeout);

    m_comPort_timer = new QTimer();
    m_comPort_timer->setInterval(1000);
    connect(m_comPort_timer, &QTimer::timeout, this, &Widget::slot_comPort_timeout);
    m_comPort_timer->start();

    m_serialPort = new QSerialPort();
    connect(m_serialPort, &QSerialPort::readyRead, this, &Widget::slot_readyRead);


    //----------------------------------------------------------------------
    m_smtTool = new SMTTool();
    connect(m_smtTool, &SMTTool::sig_sendBuf, this, &Widget::slot_sendData);

    //-------
    init();

    #ifdef SELF_TEST_ENABLE
        test_init();
    #endif
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

void Widget::init()
{
    smt_run_time = 0;

    connect(this, &Widget::sig_dispUpdata, this, &Widget::slot_dispUpdata);

    m_tableWidget_config = nullptr;
    m_tableWidget_home   = nullptr;

    m_compareTypeList << "int" << "string" << "bool" << "flag" << "io";
    //--------------------------------------------------------
    loadConfig();
    dispConfigPage();
    saveConfig();
    dispHomePage();
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
    if(!m_serialPort->isOpen() || !m_smt_timer->isActive())
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

    emit sig_dispUpdata();
}
#endif

void Widget::slot_numCheck_stateChanged(int state)
{   
    //checked=2 , unchecked=0
    ui->groupBox_numCheck->setVisible((state==2)?true:false);

    string node_name = ui->label_IMEI->text().toStdString();
    item_t item = m_smtTool->home_map[node_name];
    item.value = "";
    m_smtTool->home_map[node_name] = item;
}

void Widget::slot_autoStop_stateChanged(int state)
{
    qDebug() << "autoStop: " << state;
}

void Widget::slot_IMEI_changed(QString text)
{
    string imei_value = text.toStdString();
    item_t item = m_smtTool->home_map["IMEI"];
    item.value = imei_value;
    m_smtTool->home_map["IMEI"] = item;

    qDebug() << "home_map new imei = " << text;
}

void Widget::on_pushButton_open_clicked()
{
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);

    //m_serialPort->setReadBufferSize(91);//91
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

        m_smtTool->start();//write start data
        m_smtTool->smt_state = START;//start

        smt_run_time = 0;
        m_smt_timer->start();//smt running
        ui->label_spendTime->setText(QString::number(0));

        ui->label_result->setText("testing");

        #ifdef SELF_TEST_ENABLE
            m_test_timer->start();
        #endif
    }
    else if(ui->pushButton_smt->text() == QString("停止测试"))
    {
        ui->pushButton_smt->setText(QString("SMT测试"));

        m_smtTool->stop();//write stop data
        m_smtTool->smt_state = STOP;//stop
        //m_smt_timer->stop();

        #ifdef SELF_TEST_ENABLE
            m_test_timer->stop();
        #endif
    }
}

void Widget::slot_smt_timeout()
{
    static int start_timeout = 0;
    static int stop_timeout = 0;

    switch(m_smtTool->smt_state)
    {
        case START:
        {
            m_smtTool->start();
            smt_run_time++;
            start_timeout++;
            stop_timeout = 0;
        }
        break;
        case STOP:
        {
            m_smtTool->stop();
            start_timeout = 0;
            stop_timeout++;
        }
        break;
        case RUNING:
        {
            smt_run_time++;
            start_timeout = 0;
        }
        break;
        case STOPING:
        {
            stop_timeout = 0;
        }
        break;
    }

    //communicate error timeout
    if(start_timeout >= 10 || stop_timeout >= 5)
    {
        if(m_smtTool->smt_state == START)
        {
            ui->label_result->setText(QString("com error"));
            emit ui->pushButton_smt->clicked(); //auto stop
        }
        else if(m_smtTool->smt_state == STOP)
        {
            m_smtTool->smt_state = STOPING;
        }
    }

    //---------------------------------------------------------
    //smt_run_time++;
    ui->label_spendTime->setText(QString::number(smt_run_time));
}

void Widget::slot_sendData(unsigned char *data, int len)
{
    m_serialPort->write(reinterpret_cast<char*>(data), len);
}

void Widget::on_toolButton_config_clicked()
{
    if(ui->toolButton_config->text() == QString("配置"))
    {
        ui->toolButton_config->setText(QString("保存"));

        //qDebug() << "loadConfig  --> display config page";
        loadConfig();
        dispConfigPage();
    }
    else if(ui->toolButton_config->text() == QString("保存"))
    {
       ui->toolButton_config->setText(QString("配置"));

       //qDebug() << "save config --> display home page";
       saveConfig();
       dispHomePage();
    }
}

void Widget::slot_readyRead()
{
    QByteArray rxArray = m_serialPort->readAll();
    if(!rxArray.isEmpty())
    {
        m_rxArray.append(rxArray);
        if(m_rxArray.size() >= 91)
        {
            //msg process
            //m_smtTool->msg_process((unsigned char*)rxArray.data(), rxArray.size());
            int result = m_smtTool->msg_process(reinterpret_cast<unsigned char*>(rxArray.data()), rxArray.size());
            if(result == -1 || result == 1) // -1: data error; 1: data right; 0:part data
            {
                m_rxArray.clear();
            }

            emit sig_dispUpdata();
        }
    }
}

void Widget::slot_dispUpdata()
{
    // display frame_map value according to home_map key, then compare to home_map value
    int row_cnt = m_tableWidget_home->rowCount();
    for(int i=0; i<row_cnt; i++)
    {
        //tableWidget --> node name -->value = frame_map[node_name] --> tableWidget item value
        QTableWidgetItem *p_item_c0 = m_tableWidget_home->item(i, 0);
        QTableWidgetItem *p_item_c1 = m_tableWidget_home->item(i, 1);
        QTableWidgetItem *p_item_c2 = m_tableWidget_home->item(i, 2);

        string node_name = p_item_c0->text().toStdString();
        string new_value = m_smtTool->frame_map[node_name];
        p_item_c1->setText(QString::fromStdString(new_value));

        string set_value = m_smtTool->home_map[node_name].value;
        string compareType = m_smtTool->home_map[node_name].type;

        //compare() --> result
        QString q_new_value = QString::fromStdString(new_value);
        QString q_set_value = QString::fromStdString(set_value);
        QString q_type      = QString::fromStdString(compareType);
        bool result = valueCompare(q_new_value, q_set_value, q_type);

        p_item_c2->setText(QString((result==0)?"fail":"pass"));
        p_item_c2->setBackgroundColor((result==0)?Qt::red:Qt::green);
    }

    //-----------------------------------------------------------------
    //check all result , if all pass, auto stop;

}

bool Widget::valueCompare(QString value, QString set_value, QString type)
{
    if(type == "int")
    {
        if(set_value.isEmpty())
        {
            return (value.isEmpty()?false:true);
        }
        else
        {
           return (value.toInt()>set_value.toInt())?true:false;
        }
    }
    else if(type == "string")
    {
        if(set_value.isEmpty())
        {
            return (value.isEmpty()?false:true);
        }
        else
        {
            return (value==set_value)?true:false;
        }
    }
    else if(type == "flag" || type == "bool")
    {
        return (value.toInt()?true:false);
    }

    return false;
}

void Widget::loadConfig()
{
    //config.yaml --> config_map --> tableWidget
    ifstream f_in("./config.yaml");
    if(!f_in)
    {
        qDebug() << "read config.yaml error";
        return;
    }

    YAML::Node config = YAML::Load(f_in);
    if(!config.IsDefined()) //if(config.IsNull())
    {
        qDebug() << "yaml load error";
        goto LOAD_END;
    }

    m_smtTool->config_map.clear();
    for(YAML::const_iterator it=config.begin(); it!=config.end(); it++)
    {
        string node_name = it->first.as<string>();
        string enable;
        string type;
        string value;
        if(config[node_name]["enable"])
        {
            enable = config[node_name]["enable"].as<string>();
        }
        if(config[node_name]["type"])
        {
            type = config[node_name]["type"].as<string>();
            if(config[node_name]["value"])
            {
                value = config[node_name]["value"].as<string>();
            }
        }

        //cout << node_name << ":" << enable << "," << type << "," << value << endl;
        //------------------------------------------------------------------------
        m_smtTool->config_map[node_name].enable = enable;
        m_smtTool->config_map[node_name].type   = type;
        m_smtTool->config_map[node_name].value  = value;
    }

    LOAD_END:
    f_in.close();
}

void Widget::dispConfigPage()
{
    //config table widget
    if(!m_tableWidget_config)
    {
        m_tableWidget_config = new QTableWidget();
        m_tableWidget_config->setColumnCount(CONFIG_COL_CNT);
        QStringList headerList;
        headerList << QString("测试项") << QString("设置值") << QString("比较类型");
        m_tableWidget_config->setHorizontalHeaderLabels(headerList);
        m_tableWidget_config->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_tableWidget_config->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        //ui->stackedWidget->insertWidget(CONFIG_WIDGET_INDEX,m_tableWidget_config);
        ui->stackedWidget->addWidget(m_tableWidget_config);
    }
    else
    {
        m_tableWidget_config->clearContents();
        for(int i=m_tableWidget_config->rowCount(); i>0; i--) //!!!
        {
            m_tableWidget_config->removeRow(i-1);
        }
    }
    //ui->stackedWidget->setCurrentIndex(CONFIG_WIDGET_INDEX);
    ui->stackedWidget->setCurrentWidget(m_tableWidget_config);
    //---------------------------------------------------------------------------------
    map<string, item_t>::iterator iter;
    int row = 0;
    for(iter = m_smtTool->config_map.begin(); iter!=m_smtTool->config_map.end(); iter++, row++)
    {
        string c0_text(iter->first);
        string c0_enable(iter->second.enable);
        string c1_text(iter->second.value);
        string c2_text(iter->second.type); //maybe combox

        QTableWidgetItem *p_item_c0 = new QTableWidgetItem();
        QTableWidgetItem *p_item_c1 = new QTableWidgetItem();
        QComboBox *p_item_c2 = new QComboBox();

        Qt::ItemFlags flags = p_item_c0->flags();
        flags &= ~Qt::ItemIsEditable;
        p_item_c0->setFlags(flags);
        p_item_c0->setCheckState((c0_enable=="0"?Qt::Unchecked:Qt::Checked));
        p_item_c0->setText(QString::fromStdString(c0_text));
        p_item_c1->setText(QString::fromStdString(c1_text));
        p_item_c2->addItems(m_compareTypeList);
        p_item_c2->setCurrentText(QString::fromStdString(c2_text));

        m_tableWidget_config->insertRow(row);
        m_tableWidget_config->setItem(row, 0, p_item_c0);
        m_tableWidget_config->setItem(row, 1, p_item_c1);
        m_tableWidget_config->setCellWidget(row, 2, p_item_c2);
    }
}

void Widget::saveConfig()
{
    //clear home_map --> tableWidget --> config.yaml && home_map
    ofstream f_out("./config.yaml");
    if(!f_out)
    {
        qDebug() << "open config.yaml error";
    }
    YAML::Node config;

    m_smtTool->home_map.clear();
    int row_cnt = m_tableWidget_config->rowCount();
    for(int i=0; i<row_cnt; i++)
    {
        QTableWidgetItem *p_item_c0 = m_tableWidget_config->item(i, 0);
        QTableWidgetItem *p_item_c1 = m_tableWidget_config->item(i, 1);
        QComboBox *p_item_c2 = (QComboBox*)m_tableWidget_config->cellWidget(i, 2);

        string node_name = p_item_c0->text().toStdString();
        int enable = (p_item_c0->checkState()==Qt::Checked?1:0);
        string value = p_item_c1->text().toStdString();
        string type = p_item_c2->currentText().toStdString();

        if(enable) //-->home_map
        {
            item_t item = {to_string(enable), type, value};
            m_smtTool->home_map[node_name] = item;
        }

        //--->config.yaml
        config[node_name]["enable"] = (enable);
        config[node_name]["type"] = type;
        config[node_name]["value"] = value;
    }

    f_out << config;
    f_out.close();
}

void Widget::dispHomePage()
{
    if(!m_tableWidget_home)
    {
        m_tableWidget_home = new QTableWidget();
        m_tableWidget_home->setColumnCount(HOME_COL_CNT);
        QStringList headerList;
        headerList << QString("测试项") << QString("值") << QString("测试结果");
        m_tableWidget_home->setHorizontalHeaderLabels(headerList);
        m_tableWidget_home->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_tableWidget_home->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        //ui->stackedWidget->insertWidget(HOME_WIDGET_INDEX,m_tableWidget_home);
        ui->stackedWidget->addWidget(m_tableWidget_home);
    }
    else
    {
        m_tableWidget_home->clearContents();
        for(int i=m_tableWidget_home->rowCount(); i>0; i--) //!!!
        {
            m_tableWidget_home->removeRow(i-1);
        }
    }
    //ui->stackedWidget->setCurrentIndex(HOME_WIDGET_INDEX);
    ui->stackedWidget->setCurrentWidget(m_tableWidget_home);
    //-------------------------------------------------------------------------------
    map<string, item_t>::iterator iter;
    int row=0;
    for(iter=m_smtTool->home_map.begin(); iter != m_smtTool->home_map.end(); iter++, row++)
    {
        string c0_text(iter->first);
        item_t item = iter->second;//

        m_tableWidget_home->insertRow(row);

        //item
        QTableWidgetItem *p_item_c0 = new QTableWidgetItem();
        p_item_c0->setText(QString::fromStdString(c0_text));
        Qt::ItemFlags flags = p_item_c0->flags();
        flags &= ~Qt::ItemIsEditable;
        p_item_c0->setFlags(flags);
        m_tableWidget_home->setItem(row, 0, p_item_c0);

        //result
        QTableWidgetItem *p_item_c2 = new QTableWidgetItem();
        p_item_c2->setFlags(flags);
        m_tableWidget_home->setItem(row, 2, p_item_c2);

        //value display
        if(item.type != "io")
        {
            QTableWidgetItem *p_item_c1 = new QTableWidgetItem();
            p_item_c1->setFlags(flags);
            m_tableWidget_home->setItem(row, 1, p_item_c1);
        }
        else
        {
            QTableWidget *p_item_c1 = new QTableWidget();
            p_item_c1->setRowCount(4);
            p_item_c1->setColumnCount(5);

            p_item_c1->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            p_item_c1->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

            m_tableWidget_home->setCellWidget(row, 1, p_item_c1);
        }
    }
}




