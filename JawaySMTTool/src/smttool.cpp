#include "smttool.h"
#include <iostream>
#include <string.h>
#include <stdio.h>

SMTTool::SMTTool(QObject *parent) : QObject(parent)
{

}

SMTTool::~SMTTool()
{

}

void SMTTool::start()
{
    emit sig_sendBuf(const_cast<unsigned char*>(bufStart), START_BUF_SIZE);
}

void SMTTool::stop()
{
    emit sig_sendBuf(const_cast<unsigned char*>(bufStop), STOP_BUF_SIZE);
}

int SMTTool::msg_process(unsigned char *data, int len)
{

    //0xB6 6B FA AA 53 (84字节结果) 00 0D    //size = 91
    unsigned char head[] = {0xb6, 0x6b, 0xfa, 0xaa, 0x06};
    unsigned char tail[] = {0x00, 0x0d};

    unsigned char *array = data;
    if(array[0] == head[0] && array[1] == head[1] && array[2] == head[2] && array[3] == head[3] && array[4] == head[4] &&
       array[len-1] == tail[1] && array[len-2] == tail[0])
    {
        smt_state = RUNING;// receive valid data, can stop smt_timer
        //msg_process(m_rxArray.data(), m_rxArray.size());

    }

    return 0;
}
