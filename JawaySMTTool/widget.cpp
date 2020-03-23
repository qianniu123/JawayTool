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
    this->setWindowTitle(QString("嘉为SMT测试工具_V2.03"));//JawaySMTTool_V2.0

    //--------------------------------------------------------------------------------------------------
    ui->pushButton_smt->setEnabled(false);

    connect(ui->checkBox_numCheck, &QCheckBox::stateChanged, this, &Widget::slot_numCheck_stateChanged);
    connect(ui->checkBox_autoStop, &QCheckBox::stateChanged, this, &Widget::slot_autoStop_stateChanged);
    ui->checkBox_autoStop->setCheckState(Qt::Checked);
    ui->checkBox_numCheck->setCheckState(Qt::Unchecked); 

    ui->frame->setLayout(ui->verticalLayout_frame);
    ui->frame->setVisible(false);

    connect(ui->lineEdit_IMEI, &QLineEdit::textChanged, this, &Widget::slot_IMEI_changed);

    QFont font;
    font.setBold(true);
    font.setPixelSize(30);
    ui->label_model->setFont(font);
    ui->label_model->setAlignment(Qt::AlignCenter);

    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::red);
    font.setBold(false);
    font.setPixelSize(15);
    ui->label_model_type_error->setPalette(pe);
    ui->label_model_type_error->setFont(font);
    ui->label_model_type_error->setAlignment(Qt::AlignCenter);
    ui->label_model_type_error->setVisible(false);
    //----------------------------------------------------------------------
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

    connect(this, &Widget::sig_dispUpdate, this, &Widget::slot_dispUpdate);
    connect(this, &Widget::sig_dispUpdate_io, this, &Widget::slot_dispUpdate_io);

    m_tableWidget_config = nullptr;
    m_tableWidget_home   = nullptr;

    m_compareTypeList << "int" << "string" << "bool" << "flag" << "io";

    ui->comboBox_matnr->insertItems(0, m_smtTool->matnr_strlist);
    connect(ui->comboBox_matnr, &QComboBox::currentTextChanged, this, &Widget::slot_combox_matnr_changed);
    //-----------------------------------------------------------------
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
    m_smtTool->frame_map["IMEI"] = "123456789";
    m_smtTool->frame_map["LOCK"] = "0";
    m_smtTool->frame_map["外电"] = "1";
    m_smtTool->frame_map["电门"] = "0";

    m_smtTool->frame_map["MODEL"] = "21";
    m_smtTool->frame_map["DEVICE_TYPE"] = "1";
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
    ui->frame->setVisible((state==2)?true:false);
    if(state == 2)//checked //package test
    {
        ui->lineEdit_IMEI->setFocus(); //forcus

        item_t imei_item = m_smtTool->home_map["IMEI"];
        imei_item.value = "null";
        m_smtTool->home_map["IMEI"] = imei_item;
    }
    else if(state == 0) //SMT test
    {
        ui->lineEdit_IMEI->clear();

        item_t imei_item = m_smtTool->home_map["IMEI"];
        imei_item.value = "";
        m_smtTool->home_map["IMEI"] = imei_item;
    }

    //-----------------------------------------------------------------------------------------------
    //clear imei & model test result
    if(ui->pushButton_smt->text() == QString("停止测试"))
    {
        ui->label_model->clear();
        m_smtTool->result_map["IMEI"] = 0;
        QList<QTableWidgetItem*> p_item_list = m_tableWidget_home->findItems("IMEI", Qt::MatchFixedString);
        if(!p_item_list.isEmpty())
        {
            QTableWidgetItem* p_item_c0 = p_item_list.at(0);
            int row_index = p_item_c0->row();
            QTableWidgetItem* p_item_c2 = m_tableWidget_home->item(row_index, 2);
            p_item_c2->setBackgroundColor(Qt::red);
            p_item_c2->setText(QString("fail"));
        }
    }
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
    //------------------------------------------------------------------------------------------------
    //clear old imei result  //when test is stoped, alse clear the result!
    if(ui->pushButton_smt->text() == QString("停止测试"))//(m_smt_timer && m_smt_timer->isActive()) //if is testing
    {
        m_smtTool->result_map["IMEI"] = false;
        QList<QTableWidgetItem*> p_item_list = m_tableWidget_home->findItems("IMEI", Qt::MatchFixedString);
        if(!p_item_list.isEmpty())
        {
            QTableWidgetItem* p_item_c0 = p_item_list.at(0);
            int row_index = p_item_c0->row();
            QTableWidgetItem* p_item_c2 = m_tableWidget_home->item(row_index, 2);
            p_item_c2->setBackgroundColor(Qt::red);
            p_item_c2->setText(QString());
        }
    }
}

void Widget::slot_combox_matnr_changed(QString text)
{
    //clear old label_model result
    if(ui->pushButton_smt->text() == QString("停止测试"))
    {
        ui->label_model->clear();
    }
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

        m_smtTool->start();//write start data
        m_smtTool->smt_state = START;//start

        smt_run_time = 0;
        m_smt_timer->start();//smt running
        ui->label_spendTime->setText(QString::number(0));        

        dispHomePage();//clear the value and result
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

        //if num checkBox enabled; set focus in IMEI lineEdit
        if(ui->checkBox_numCheck->isChecked())
        {
            ui->lineEdit_IMEI->clear();
            ui->lineEdit_IMEI->setFocus();
        }

        //save log
        slot_saveLog();
    }
}

void Widget::slot_smt_timeout()
{
    static int start_timeout = 0;
    static int stop_timeout = 0;
    static QString test_state;

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
            ui->label_model->setText(test_state);
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
    //m_serialPort->waitForReadyRead(100);

    static bool frame_head_flag = false;
    if(!rxArray.isEmpty())
    {
        m_rxArray.append(rxArray);
        while(!frame_head_flag && m_rxArray.length()>2)//check frame head
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
            int result = m_smtTool->msg_process(frame_buff, frame_len);
            if(result == -1 || result == 1) // -1: data error; 1: data right; 0:part data
            {
                //qDebug() << "-------------get a frame, and clear-------------: result = " << result;
                m_rxArray.clear();
                frame_head_flag = false;
            }

            emit sig_dispUpdate();
        }
    }
}

void Widget::slot_dispUpdate()
{
    //if have GPIO
    emit sig_dispUpdate_io();

    // display frame_map value according to home_map key, then compare to home_map value
    int row_cnt = m_tableWidget_home->rowCount();
    for(int i=0; i<row_cnt; i++)
    {
        //tableWidget --> node name -->value = frame_map[node_name] --> tableWidget item value
        QTableWidgetItem *p_item_c0 = m_tableWidget_home->item(i, 0);
        if(p_item_c0 == nullptr) continue;
        string node_name = p_item_c0->text().toStdString();
        if(m_smtTool->home_map[node_name].type == "io")
        {
            continue;// do not update io type
        }

        QTableWidgetItem *p_item_c1 = m_tableWidget_home->item(i, 1);
        QTableWidgetItem *p_item_c2 = m_tableWidget_home->item(i, 2);

        string new_value = m_smtTool->frame_map[node_name];
        p_item_c1->setText(QString::fromStdString(new_value));

        string set_value = m_smtTool->home_map[node_name].value;
        string compareType = m_smtTool->home_map[node_name].type;

        //compare() --> result
        QString q_new_value = QString::fromStdString(new_value);
        QString q_set_value = QString::fromStdString(set_value);
        QString q_type      = QString::fromStdString(compareType);
            //for test
            //qDebug()<<"node_name="<<p_item_c0->text()<<";type="<<q_type;
            //qDebug()<<"value="<<q_new_value<<";set_value="<<q_set_value;
            //qDebug()<< "value_len=" << q_new_value.length();
        bool result = valueCompare(q_new_value, q_set_value, q_type);

        p_item_c2->setText(QString((result==0)?"fail":"pass"));
        p_item_c2->setBackgroundColor((result==0)?Qt::red:Qt::green);

        //set result
        m_smtTool->result_map[node_name] = result;

    }

    //-----------------------------------------
    //display model MATNR and check devicetype
    slot_dispUpdate_model();
    //----------------------------------------------------------------------------------
    //check result_map , if all pass, --> auto stop; --> and save log
    map<string,bool>::iterator iter;
    for(iter=m_smtTool->result_map.begin(); iter!=m_smtTool->result_map.end(); iter++)
    {
        if(iter->second == false)
        {
            return;
        }
    }
    //all pass, auto stop
    if(ui->checkBox_autoStop->isChecked() && ui->pushButton_smt->text() == QString("停止测试"))
    {
        emit ui->pushButton_smt->click();
    }   
}

void Widget::slot_dispUpdate_io()
{
#if (GPIO_DISPLAY_MODE == MODE_REGULAR)
    // display frame_map value according to home_map key, then compare to home_map value
    if(m_tableWidget_home->columnCount() == HOME_COL_CNT) //no io pin
    {
        return;
    }

    int row_cnt = m_tableWidget_home->rowCount();
    for(int i=0; i<row_cnt; i++)
    {
        //tableWidget --> node name -->value = frame_map[node_name] --> tableWidget item value
        QTableWidgetItem *p_item_c3 = m_tableWidget_home->item(i, 3);
        if(p_item_c3 == nullptr)
        {
            continue;
        }
        QTableWidgetItem *p_item_c4 = m_tableWidget_home->item(i, 4);

        string node_name = p_item_c3->text().toStdString();
        string new_value = m_smtTool->frame_map[node_name];
        p_item_c4->setText(QString::fromStdString(new_value));

        // no need compare -->result
        bool result = (new_value=="1")?true:false;
        //p_item_c4->setText(QString((result==0)?"fail":"pass"));
        p_item_c4->setText(QString::fromStdString(new_value));// for test
        p_item_c4->setBackgroundColor((result==0)?Qt::red:Qt::green);

        //set result
        m_smtTool->result_map[node_name] = result;
    }
#elif (GPIO_DISPLAY_MODE == MODE_TABLE)
    if(m_smtTool->home_map.find("GPIO") == m_smtTool->home_map.end()) //not exist node node "GPIO"
    {
        return;
    }
    //find gpio table
    QList<QTableWidgetItem*> item_list = m_tableWidget_home->findItems(QString("GPIO"), Qt::MatchExactly);
    if(item_list.isEmpty())
    {
        return;
    }
    int cnt = item_list.count();
    QTableWidgetItem *p_item_io = item_list.first();
    int row_index = p_item_io->row();
    int col_index =p_item_io->column();
    qDebug() << "find gpio tableWidget;" << "cnt = " << cnt << "; node_name = " << p_item_io->text() << "; row_index = " << row_index;

    //------------------------------------------------------------------------------------------------
    QTableWidget *p_table_io = (QTableWidget*)m_tableWidget_home->cellWidget(row_index, col_index+1);

    map<string, item_t>::iterator iter;
    for(iter=m_smtTool->home_map_io.begin(); iter!=m_smtTool->home_map_io.end(); iter++)
    {
        string pin_name = iter->first;
        item_t pin_item = iter->second;
        QList<QTableWidgetItem*> io_item_list = p_table_io->findItems(QString::fromStdString(pin_name), Qt::MatchExactly);
        if(item_list.isEmpty())
        {
            continue;
        }
        QTableWidgetItem *p_item = io_item_list.first();
        int row_index_io = p_item->row();
        int col_index_io = p_item->column();
        // updata pin value
        string pin_newValue = m_smtTool->frame_map[pin_name];//? why on value
        p_table_io->item(row_index_io+1, col_index_io)->setText(QString::fromStdString(pin_newValue));
        p_table_io->item(row_index_io+1, col_index_io)->setBackgroundColor((pin_newValue=="1"?Qt::green:Qt::red));
//        qDebug() << "pin_name = " << QString::fromStdString(pin_name) << "; row = "<< row_index_io << "; col = " << col_index_io \
//                 << "pin_value = " << QString::fromStdString(pin_newValue);
    }

    //-----------------------------------------------------------------
    //check result , if all io pass, --> set result_map["GPIO"] = true
    for(iter=m_smtTool->home_map_io.begin(); iter!=m_smtTool->home_map_io.end(); iter++)
    {
        string pin_name = iter->first;
        item_t pin_item = iter->second;
        QList<QTableWidgetItem*> io_item_list = p_table_io->findItems(QString::fromStdString(pin_name), Qt::MatchExactly);
        if(item_list.isEmpty())
        {
            continue;
        }
        QTableWidgetItem *p_item = io_item_list.first();
        int row_index_io = p_item->row();
        int col_index_io = p_item->column();
        // read pin value
        QString pin_result = p_table_io->item(row_index_io+1, col_index_io)->text();
        if(pin_result.toInt() == 0)
        {
            return;
        }
    }
    //----
    m_smtTool->result_map["GPIO"] = true;

#endif
}

void Widget::slot_dispUpdate_model()
{
    //display model MATNR and check devicetype(package test vs smt test)
    QString model_str = QString::fromStdString(m_smtTool->frame_map["MODEL"]);
    uint16_t model = model_str.toUShort();
    QString deviceType_str = QString::fromStdString(m_smtTool->frame_map["DEVICE_TYPE"]);
    uint8_t deviceType = (uint8_t)deviceType_str.toUInt();
    QString matnr;
    QString jaway_config;
    auto second = m_smtTool->model_matnr_map[model];
    if(second.size())
    {
        matnr = QString::fromStdString(second.begin()->first);
        jaway_config = QString::fromStdString(second.begin()->second);
    }

    QPalette pe_red;
    pe_red.setColor(QPalette::WindowText, Qt::red);
    QPalette pe_black;
    pe_black.setColor(QPalette::WindowText, Qt::black);

    if(ui->checkBox_numCheck->isChecked()) //package test
    {
        QString matnr_tmp = ui->comboBox_matnr->currentText(); //get matnr from combo_matnr
        if(!matnr.isEmpty())
        {
            ui->label_model->setText(QString("%1(%2)").arg(matnr).arg(jaway_config));
            if(matnr_tmp != matnr)
            {
                ui->label_model->setPalette(pe_red);
            }
            else
            {
                ui->label_model->setPalette(pe_black);
            }
        }
        else
        {
            ui->label_model->setPalette(pe_red);
            ui->label_model->setText(QString("MATNR error"));
        }
        //-------------
        if(m_smtTool->model_deviceType_map[model] == deviceType)
        {
            ui->label_model_type_error->setVisible(false);
        }
        else
        {
            qDebug() << "model_deviceType_map[model]="<< m_smtTool->model_deviceType_map[model];
            ui->label_model_type_error->setVisible(true);
            ui->label_model_type_error->setText(QString("型号(%1)和系统类型(%2)不匹配").arg(model).arg(deviceType));
        }
    }
    else //smt test
    {
        if(!matnr.isEmpty())
        {
            ui->label_model->setText(QString("%1(%2)").arg(matnr).arg(jaway_config));
            ui->label_model->setPalette(pe_black);
        }
        else
        {
            ui->label_model->setText(QString("no MATNR"));
            //ui->label_model->setPalette(pe_red);
        }
    }

    //qDebug() << matnr << ":" << model_str <<"("<< model<<")-->"<< deviceType_str<<"("<<deviceType<<")";
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
    // config.yml --> config_map
    m_smtTool->loadConfig();
}

//config_map --> configPage
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
        m_tableWidget_config->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//Stretch//ResizeToContents
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

//save configPage to config_map, refresh home_map, result_map
void Widget::saveConfig()
{
    //clear home_map --> tableWidget --> home_map && config_map && result_map
    m_smtTool->home_map.clear();
    m_smtTool->result_map.clear();
    //m_smtTool->config_map.clear();

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
            if(type == "io") //node_name = "GPIO", type="io", value="[00,01,02,12,13]"
            {
                //set gpio home_map
                #if (GPIO_DISPLAY_MODE == MODE_REGULAR)
                    setGpioList(value);  //config_map value --> pin name
                #elif (GPIO_DISPLAY_MODE == MODE_TABLE)
                    item_t item = {to_string(enable), type, value};
                    m_smtTool->home_map[node_name] = item;
                    m_smtTool->result_map[node_name] = 0;
                    //init gpio_list
                    setGpioList(value); //!!!!!!
                #endif
            }
            else
            {
                item_t item = {to_string(enable), type, value};
                m_smtTool->home_map[node_name] = item;
                m_smtTool->result_map[node_name] = 0;
            }
        }

        //--->config_map
        item_t item = {to_string(enable), type, value};
        m_smtTool->config_map[node_name] = item;
    }

    //config_map --> config.yml
    m_smtTool->saveConfig();
}

void Widget::setGpioList(string gpios)
{
    //gpio ---> home_map
    //str = "[0,1,2,3,4,5,6,10,11,12,13,21,31,32,33,34,36,40,46,47,48]"
    //str = "[P00,P01,P02,P03,P04,P05,P06,P10,P11,P12,P13,GPIO21,GPIO31,GPIO32,GPIO33,GPIO34,GPIO36,GPIO40,GPIO46,GPIO47,GPIO48]"
    QString pinstr = QString::fromStdString(gpios);
    pinstr.remove(QChar('['));
    pinstr.remove(QChar(']'));
    QStringList pin_list = pinstr.split(',', QString::SkipEmptyParts);
    //for(QString &pin:pin_list)
    //{
    //   pin.insert(0, 'P');
    //}
    //int pin_cnt = pin_list.count();
    //-----------------------------------------------------------------
    //use pin_list set vector<string> gpio_list in frame_map
    m_smtTool->gpio_list.clear();
    #if (GPIO_DISPLAY_MODE == MODE_REGULAR)
    for(QString pin_name:pin_list)
    {
        item_t item = {to_string(1), "io", ""};
        string node_name = pin_name.toStdString();
        m_smtTool->home_map[node_name] = item;
        m_smtTool->result_map[node_name] = 0;
        m_smtTool->gpio_list.push_back(node_name);//gpio list in frame_map
    }
    #elif (GPIO_DISPLAY_MODE == MODE_TABLE)
    for(QString pin_name:pin_list)
    {
        string node_name = pin_name.toStdString();
        m_smtTool->gpio_list.push_back(node_name);//gpio list in frame_map
    }

    //item_t item = {to_string(enable), type, value};
    //m_smtTool->home_map[node_name] = item;
    #endif
}

void Widget::dispHomePage() //init homepage display
{
    if(!m_tableWidget_home)
    {
        m_tableWidget_home = new QTableWidget();
        m_tableWidget_home->setColumnCount(HOME_COL_CNT);
        QStringList headerList;
        headerList << QString("测试项") << QString("值") << QString("测试结果");
        m_tableWidget_home->setHorizontalHeaderLabels(headerList);
        //m_tableWidget_home->setHorizontalHeaderItem();
        m_tableWidget_home->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//Stretch//ResizeToContents
        m_tableWidget_home->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        ui->stackedWidget->addWidget(m_tableWidget_home);
    }
    else
    {
        m_tableWidget_home->clearContents();
        for(int i=m_tableWidget_home->rowCount(); i>0; i--) //!!!
        {
            m_tableWidget_home->removeRow(i-1);
        }
        if(m_tableWidget_home->columnCount() != HOME_COL_CNT)
        {
            m_tableWidget_home->setColumnCount(HOME_COL_CNT);
        }

        ui->label_model->clear();
        ui->label_model_type_error->setVisible(false);
    }
    ui->stackedWidget->setCurrentWidget(m_tableWidget_home);
    //------------------------------------------------------------------------------------
    map<string, item_t>::iterator iter;
    int row=0;
    #if (GPIO_DISPLAY_MODE == MODE_TABLE)

    #elif (GPIO_DISPLAY_MODE == MODE_REGULAR)
        int pin_row = 0;///!!!
    #endif
    for(iter=m_smtTool->home_map.begin(); iter != m_smtTool->home_map.end(); iter++, row++)
    {
        string c0_text(iter->first);
        item_t item = iter->second;//

        if(item.type == "io")
        {
            #if (GPIO_DISPLAY_MODE == MODE_TABLE)
                if(m_tableWidget_home->rowCount()-1 < row)
                {
                    m_tableWidget_home->insertRow(row);
                }
                //node name
                QTableWidgetItem *p_item_c0 = new QTableWidgetItem();
                p_item_c0->setText(QString::fromStdString(c0_text));
                Qt::ItemFlags flags = p_item_c0->flags();
                flags &= ~Qt::ItemIsEditable;
                p_item_c0->setFlags(flags);
                m_tableWidget_home->setItem(row, 0, p_item_c0);
                //gpio table
                QString gpios = QString::fromStdString(item.value);
                dispGpioTable(row, 1, gpios);
            #elif (GPIO_DISPLAY_MODE == MODE_REGULAR)     
                dispGpioTable(c0_text, item, pin_row);
                row--;  ///!!!
                pin_row++; // add pin_row, not row
            #endif
        }
        else
        {
            if(m_tableWidget_home->rowCount()-1 < row)
            {
                m_tableWidget_home->insertRow(row);
            }
            //node name
            QTableWidgetItem *p_item_c0 = new QTableWidgetItem();
            p_item_c0->setTextAlignment(Qt::AlignHCenter);
            p_item_c0->setText(QString::fromStdString(c0_text));
            Qt::ItemFlags flags = p_item_c0->flags();
            flags &= ~Qt::ItemIsEditable;
            p_item_c0->setFlags(flags);
            m_tableWidget_home->setItem(row, 0, p_item_c0);
            //value & result
            QTableWidgetItem *p_item_c1 = new QTableWidgetItem();
            p_item_c1->setTextAlignment(Qt::AlignHCenter);
            p_item_c1->setFlags(flags);
            m_tableWidget_home->setItem(row, 1, p_item_c1);
            QTableWidgetItem *p_item_c2 = new QTableWidgetItem();
            p_item_c2->setTextAlignment(Qt::AlignHCenter);
            p_item_c2->setFlags(flags);
            m_tableWidget_home->setItem(row, 2, p_item_c2);
        }
    }
}

void Widget::dispGpioTable(string pin_name, item_t item, int pin_row) //MODE_REGULAR
{
    //regular display format
    if(m_tableWidget_home->columnCount() == HOME_COL_CNT)
    {
        m_tableWidget_home->setColumnCount(HOME_COL_CNT + 2);
        QStringList headerList;
        headerList << QString("测试项") << QString("值") << QString("测试结果") << QString("测试项") << QString("测试结果");
        m_tableWidget_home->setHorizontalHeaderLabels(headerList);
    }
    //----------------------------------------------------
    if(m_tableWidget_home->rowCount()-1 < pin_row)
    {
        m_tableWidget_home->insertRow(pin_row);
    }

    QTableWidgetItem *p_item_c3 = new QTableWidgetItem();
    p_item_c3->setTextAlignment(Qt::AlignHCenter);
    p_item_c3->setText(QString::fromStdString(pin_name));
    Qt::ItemFlags flags = p_item_c3->flags();
    flags &= ~Qt::ItemIsEditable;
    p_item_c3->setFlags(flags);
    m_tableWidget_home->setItem(pin_row, 3, p_item_c3);//
    //value & result
    QTableWidgetItem *p_item_c4 = new QTableWidgetItem();
    p_item_c4->setTextAlignment(Qt::AlignHCenter);
    p_item_c4->setFlags(flags);
    m_tableWidget_home->setItem(pin_row, 4, p_item_c4);
}

void Widget::dispGpioTable(int row, int col, QString gpios) //MODE_TABLE;  display io in Independent table
{
    //-----------------------------------------------------------------
    //str = "[0,1,2,3,4,5,6,10,11,12,13,21,31,32,33,34,36,40,46,47,48]"
    gpios.remove(QChar('['));
    gpios.remove(QChar(']'));
    QStringList pin_list = gpios.split(',', QString::SkipEmptyParts);
        m_smtTool->home_map_io.clear();
    for(QString &pin:pin_list)
    {
        //pin.insert(0, 'P');
        //----set home_map_io
        string node_name = pin.toStdString();
        item_t item = {"1", "io", ""};
        m_smtTool->home_map_io[node_name] = item;
    }

    int pin_cnt = pin_list.count();
    int row_cnt = 4;
    int col_cnt = pin_cnt/2; col_cnt += pin_cnt%2;
    qDebug() << "pin_cnt = " << pin_cnt  << " col_cnt = " << col_cnt;

    //-----------------------------------------------------------------------
    m_tableWidget_home->setSpan(row, col, 1, 2);//
    //span then insert tableWidget
    QTableWidget *p_item_c1 = new QTableWidget();
    p_item_c1->setRowCount(row_cnt);
    p_item_c1->setColumnCount(col_cnt);

    p_item_c1->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    p_item_c1->horizontalHeader()->setVisible(false);
    p_item_c1->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //p_item_c1->verticalHeader()->setDefaultSectionSize(10);//no use
    p_item_c1->verticalHeader()->setVisible(false);

    m_tableWidget_home->setCellWidget(row, 1, p_item_c1);
    //---------------------------------------------------------
    for(int i=0; i<row_cnt; i++)
    {
        for(int j=0; j<col_cnt; j++)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            //item->setSizeHint(QSize(30,10));//no use
            p_item_c1->setItem(i, j, item);

            int index = col_cnt*(i/2) + j;
            if(i%2==0 && index<pin_cnt)
            {              
                item->setText(pin_list.at(index));     
            }
        }
    }
}

void Widget::slot_saveLog()
{
    //系统时间 测试耗时 号码校验是否勾选 输入的imei值
    //测试项 实际值 设置值 测试结果
    QString file_name = QString("log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd"));
    QFile file(file_name);
    if (file.open(QFile::ReadWrite | QFile::Append))
    {
         QTextStream out(&file);
         //out.setFieldWidth(10);
         //out.setFieldAlignment(QTextStream::AlignLeft);
         out << "============================" << endl;
         out << QString("时间：") << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss") << endl;
         out << QString("测试耗时：")<< ui->label_spendTime->text() << endl;
         //out << QString("测试结果：") << ui->label_result->text() << endl;
        if(ui->checkBox_numCheck->isChecked())
        {
            out << QString("IMEI:") << ui->lineEdit_IMEI->text() << endl;
        }

        out << "----------------------------" << endl;
        out << QString("测试项") << "\t\t" \
            << QString("值") << "\t\t" \
            << QString("结果") << "\t" << endl;
        int rowCnt = m_tableWidget_home->rowCount();
        for(int i=0; i<rowCnt; i++)
        {
            if(m_tableWidget_home->item(i,0))
            {
                string c0_text = m_tableWidget_home->item(i,0)->text().toStdString();
                if(m_smtTool->home_map[c0_text].type == "io")
                {
                    out << m_tableWidget_home->item(i,0)->text() << "\t:" << endl;
                        //<< m_smtTool->result_map[c0_text] << endl;
                }
                else
                {
                    out << m_tableWidget_home->item(i,0)->text() << "\t:" \
                     << m_tableWidget_home->item(i,1)->text() << "\t" \
                     << m_tableWidget_home->item(i,2)->text() << endl;
                }
            }
        }
        //gpio
        if(m_tableWidget_home->item(0,3))
        {
            for(int i=0; i<rowCnt; i++)
            {
                if(m_tableWidget_home->item(i,3))
                {
                    out << m_tableWidget_home->item(i,3)->text() << "\t:" \
                     << m_tableWidget_home->item(i,4)->text() << endl;
                }
            }
        }

         //out << "===================end" << endl;
     }
}

