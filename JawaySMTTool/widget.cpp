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

    //-------------------------------------------------------------------
    connect(ui->checkBox_numCheck, &QCheckBox::stateChanged, this, &Widget::slot_numCheck_stateChanged);
    ui->checkBox_autoStop->setCheckState(Qt::Checked);
    ui->checkBox_numCheck->setCheckState(Qt::Unchecked);

    ui->groupBox_numCheck->setLayout(ui->verticalLayout_numCheck);
    ui->groupBox_numCheck->setVisible(false);
    //in horizontalLayout_sn
    ui->label_SN->setVisible(false);
    ui->lineEdit_SN->setVisible(false);




    //--------------------------------------------------------------------
    init();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::init()
{

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

}

void Widget::on_pushButton_smt_clicked()
{

}

void Widget::on_toolButton_config_clicked()
{

}
