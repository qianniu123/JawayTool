#include "smttool.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

SMTTool::SMTTool(QObject *parent) : QObject(parent)
{
    //gpio_list.push_back();
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

int SMTTool::msg_process(char *data, int len)
{
    //0xB6 6B FA AA 53 (84字节结果) 00 0D    //size = 91
    unsigned char head[] = {0xb6, 0x6b, 0xfa, 0xaa, 0x00 ,0x06};//4byte fixed, 1byte len, 1byte fixed
    unsigned char tail[] = {0x00, 0x0d}; //1byte check sum, 1byte fixed
    unsigned char *array = (unsigned char*)data;

    //cout << "-----msg_process------len="<< len << endl;
    //printf("%d,%d,%d,%d,%d,%d--%d,%d\n",array[0],array[1],array[2],array[3],array[4],array[5],array[len-2],array[len-1]);
    if(array[0] == head[0] && array[1] == head[1] && array[2] == head[2] && array[3] == head[3]
      && array[len-1] == tail[1])
    {
        smt_state = RUNING;// receive valid data, can stop smt_timer
        //int data_len = array[4];//data length(0x53=83, 0x57=87) have two types
        parse_pack(array+6, len-6-2);

        return 1;//data right
    }
    else
    {
        return -1;//data error
    }
    //return 0;
}

void SMTTool::parse_pack(unsigned char *pack, int size)
{
    //get value
    frame_t *p_frame = (frame_t*)pack;

    //save value to key-value(string, string);
    frame_map.clear();
    //type = string
    char imei[15+1] = {0};
    memcpy(imei, p_frame->imei, sizeof(p_frame->imei));
    frame_map["IMEI"] = string(imei);
    char sn[16+1] = {0};
    memcpy(sn, p_frame->sn, sizeof(p_frame->sn));
    frame_map["SN"] = string(sn);
    char softVer[8+1] = {0};
    memcpy(softVer, p_frame->softVer, sizeof(p_frame->softVer));
    frame_map["SOFTVER"] = string(softVer);
    char hardVer[8+1] = {0};
    memcpy(hardVer, p_frame->hardVer, sizeof(p_frame->hardVer));
    frame_map["HARDVER"] = string(hardVer);
    char iccid[21+1] = {0};
    memcpy(iccid, p_frame->iccid, sizeof(p_frame->iccid));
    frame_map["ICCID"] = string(iccid);
    char ble_mac[6+1] = {0};
    memcpy(ble_mac, p_frame->ble_mac, sizeof(p_frame->ble_mac));
    frame_map["BLE_MAC"] = string(ble_mac);

    //type = enum-->int-->string
    frame_map["DEVICE_TYPE"] = to_string(p_frame->deviceType);
    frame_map["MOTOR_TYPE"] = to_string(p_frame->motorType);

    //type = int-->string
    frame_map["GSM"] = to_string(p_frame->csq);
    frame_map["GPS"] = to_string(p_frame->gps_csq);
    frame_map["SNR"] = to_string(p_frame->gps_snr);
    frame_map["ADC"] = to_string(p_frame->adc);

    //type = short-->string
    frame_map["MODEL"] = to_string(*(int*)p_frame->model);

    //state -----------------------------------------------
    //type = bool-->string
    char state_flag = p_frame->state_flag;
    bool acc_flag = state_flag&0x01;
    frame_map["电门"] = to_string(acc_flag);
    bool vin_flag = (state_flag&0x02) >> 1;
    frame_map["外电"] = to_string(vin_flag);
    bool Gsensor_flag = (state_flag&0x04) >> 2;
    frame_map["Gsensor"] = to_string(Gsensor_flag);
    bool pwm_flag = (state_flag&0x08) >> 3;
    frame_map["PWM"] = to_string(pwm_flag);
    bool lock_flag = (state_flag&0x10) >> 4;
    frame_map["LOCK"] = to_string(lock_flag);
    bool muart_flag = (state_flag&0x20) >> 5;
    frame_map["MUART"] = to_string(muart_flag);
    bool adc_flag = (state_flag&0x40) >> 6;
    frame_map["ADC_FLAG"] = to_string(adc_flag);
    bool rf_flag = (state_flag&0x80) >> 7;
    frame_map["RF"] = to_string(rf_flag);

    //gpio---------------------------------------------------------------
    //printf("data size = %d, sizeof(frame_t)=%d\n", size, sizeof(frame_t));
    //type = bool-->string
    if(size == sizeof(frame_t)) //sizeof(frame_t) = 83;
    {
        return;
    }
    char gpio_flag[4]; //4 byte gpio value
    memcpy(gpio_flag, p_frame->next, sizeof(gpio_flag));
    int gpio = *(int*)gpio_flag;
        //for test //p_frame->next = 7F FF 1F 00; --> gpio=0x001FFF7F;
        //printf("gpio_flag=%02X %02X %02X %02X\n", gpio_flag[0], gpio_flag[1],gpio_flag[2],gpio_flag[3]);
        //printf("gpio=%d;%X", gpio, gpio);
    int i=0;
    for(vector<string>::iterator p_list = gpio_list.begin(); p_list != gpio_list.end(); p_list++, i++)
    {
        bool io_flag = gpio&(0x1<<i);
        frame_map[*p_list] = to_string(io_flag);
        //cout << *p_list << "\t";
    }
        //cout << endl;
}
