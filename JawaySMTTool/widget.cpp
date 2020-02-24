#include "widget.h"
#include "ui_widget.h"
#include <QCheckBox>
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
    ui->checkBox_autoStop->setCheckState(Qt::Checked);
    ui->checkBox_numCheck->setCheckState(Qt::Unchecked);

    ui->groupBox_numCheck->setLayout(ui->verticalLayout_numCheck);
    ui->groupBox_numCheck->setVisible(false);
    //in horizontalLayout_sn
    ui->label_SN->setVisible(false);
    ui->lineEdit_SN->setVisible(false);

    ui->label_result->setAlignment(Qt::AlignCenter);
    QFont result_font;
    result_font.setBold(true);
    result_font.setPixelSize(50);
    ui->label_result->setFont(result_font);

    //----------------------------------
    m_smt_timer = new QTimer();
    m_smt_timer->setInterval(1000);//ms
    connect(m_smt_timer, &QTimer::timeout, this, &Widget::slot_smt_timeout);

    m_comPort_timer = new QTimer();
    m_comPort_timer->setInterval(1000);
    connect(m_comPort_timer, &QTimer::timeout, this, &Widget::slot_comPort_timeout);
    m_comPort_timer->start();

    m_tryComm_timer = new QTimer();
    m_tryComm_timer->setInterval(1000);
    connect(m_tryComm_timer, &QTimer::timeout, this, &Widget::slot_tryComm_timeout);

    m_serialPort = new QSerialPort();
    connect(m_serialPort, &QSerialPort::readyRead, this, &Widget::slot_readyRead);


    //----------------------------------------------------------------------
    m_smtTool = new SMTTool();
    connect(m_smtTool, &SMTTool::sig_sendBuf, this, &Widget::slot_sendData);

    init();
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

}

void Widget::slot_numCheck_stateChanged(int state)
{
    if(state)
    {
        ui->groupBox_numCheck->setVisible(true);
    }
    else
    {
        ui->groupBox_numCheck->setVisible(false);
    }
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

        m_smtTool->start();
        m_smtTool->smt_state = START;//start

        smt_run_time = 0;
        m_smt_timer->start();//smt running
        ui->label_spendTime->setText(QString::number(0));
    }
    else if(ui->pushButton_smt->text() == QString("停止测试"))
    {
        ui->pushButton_smt->setText(QString("SMT测试"));

        m_smtTool->stop();
        m_smtTool->smt_state = STOP;//stop

        m_smt_timer->stop();
    }

    m_tryComm_timer->start();//通信超时定时器
}

void Widget::slot_tryComm_timeout()
{
    static int start_timeout = 0;
    static int stop_timeout = 0;

    switch(m_smtTool->smt_state)
    {
        case START:
        {
            m_smtTool->start();
            start_timeout++;
        }
        break;
        case STOP:
        {
            m_smtTool->stop();
            stop_timeout++;
        }
        break;
        case RUNING:
        case STOPING:
        {
            m_tryComm_timer->stop();
            start_timeout = 0;
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
        }
        m_smtTool->smt_state = STOPING;
    }
}

void Widget::slot_smt_timeout()
{
    smt_run_time++;

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

        loadConfig();
    }
    else if(ui->toolButton_config->text() == QString("保存"))
    {
       ui->toolButton_config->setText(QString("配置"));
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


}


void Widget::loadConfig()
{
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
        }
        if(config[node_name]["value"])
        {
            value = config[node_name]["value"].as<string>();
        }

        cout << node_name << ":" << enable << "," << type << "," << value << endl;
        //------------------------------------------------------------------------
        m_smtTool->config_map.clear();
        m_smtTool->config_map[node_name].enable = enable;
        m_smtTool->config_map[node_name].type   = type;
        m_smtTool->config_map[node_name].value  = value;
    }

    LOAD_END:
    f_in.close();
}

void Widget::dispConfigPage()
{
    size_t item_cnt = m_smtTool->config_map.size();

    //config table widget

}








