#ifndef SMTTOOL_H
#define SMTTOOL_H

#include <QObject>
#include "frame_define.h"

#define START_BUF_SIZE 9
#define STOP_BUF_SIZE 9

const unsigned char bufStart[START_BUF_SIZE] = {0xc5, 0x5c, 0xaa, 0xfa, 0x02, 0x05, 0x01, 0x00, 0x0d};
const unsigned char bufStop[STOP_BUF_SIZE]  = {0xc5, 0x5c, 0xaa, 0xfa, 0x02, 0x05, 0x00, 0x00, 0x0d};

typedef enum
{
    STOPING = 0,
    START ,
    RUNING ,
    STOP
}SMT_state_t;

class SMTTool : public QObject
{
    Q_OBJECT
public:
    explicit SMTTool(QObject *parent = nullptr);
    ~SMTTool();

    void start();
    void stop();

    SMT_state_t smt_state; //0-->1(start)-->2(runing)-->3(stop)-->0(no runing)
    int msg_process(unsigned char *data, int len);

signals:
    void sig_sendBuf(unsigned char *data, int len);

public slots:
};

#endif // SMTTOOL_H
