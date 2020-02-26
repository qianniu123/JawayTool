#include "smttool.h"
#include <iostream>
#include <string.h>
#include <stdio.h>

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

int SMTTool::msg_process(unsigned char *data, int len)
{
    //0xB6 6B FA AA 53 (84字节结果) 00 0D    //size = 91
    unsigned char head[] = {0xb6, 0x6b, 0xfa, 0xaa, 0x06};
    unsigned char tail[] = {0x00, 0x0d};

    unsigned char *array = data;
    if(array[0] == head[0] && array[1] == head[1] && array[2] == head[2] && array[3] == head[3] && array[4] == head[4]
      && array[len-1] == tail[1] && array[len-2] == tail[0])
    {
        smt_state = RUNING;// receive valid data, can stop smt_timer
        parse_pack(array+5, len-5-2);

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

    //save value to key_value;
    frame_map.clear();
    string tmp;
    tmp.append(p_frame->imei, sizeof(p_frame->imei));
    frame_map["IMEI"] = tmp;
    tmp.clear();
    tmp.append(p_frame->sn, sizeof(p_frame->sn));
    frame_map["SN"] = tmp;
    tmp.clear();
    tmp.append(p_frame->softVer, sizeof(p_frame->softVer));
    frame_map["SOFTVER"] = tmp;
    tmp.clear();
    tmp.append(p_frame->hardVer, sizeof(p_frame->hardVer));
    frame_map["HARDVER"] = tmp;
    /*
    tmp.clear();
    tmp.append(&p_frame->deviceType, sizeof(p_frame->deviceType));
    frame_map["DEVICE_TYPE"] = tmp;
    tmp.clear();
    tmp.append(&p_frame->motorType, sizeof(p_frame->motorType));
    frame_map["MOTOR_TYPE"] = tmp;
    */
    tmp.clear();
    tmp.append(p_frame->iccid, sizeof(p_frame->iccid));
    frame_map["ICCID"] = tmp;
    tmp.clear();
    tmp.append(&p_frame->csq, sizeof(p_frame->csq));
    frame_map["GSM"] = tmp;
    tmp.clear();
    tmp.append(&p_frame->gps_csq, sizeof(p_frame->gps_csq));
    frame_map["GPS"] = tmp;
    tmp.clear();
    tmp.append(&p_frame->gps_snr, sizeof(p_frame->gps_snr));
    frame_map["SNR"] = tmp;
    tmp.clear();
    tmp.append(p_frame->ble_mac, sizeof(p_frame->ble_mac));
    frame_map["BLE_MAC"] = tmp;
    tmp.clear();
    tmp.append(p_frame->model, sizeof(p_frame->model));
    frame_map["MODEL"] = tmp;
    tmp.clear();
    tmp.append(&p_frame->adc, sizeof(p_frame->adc));
    frame_map["ADC"] = tmp;
    tmp.clear();

    //state -----------------------------------
    state_flag = p_frame->state_flag;
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

    //gpio-----------------------------------------------------------------------------------------
    if(size == sizeof(frame_t))
    {
        return;
    }
    memcpy(gpio_flag, p_frame->next, sizeof(gpio_flag));
    int gpio = *(int*)gpio_flag;
    int i=0;
    //for(list<string>::iterator p_list = gpio_list.begin(); p_list != gpio_list.end(); p_list++, i++)
    for(vector<string>::iterator p_list = gpio_list.begin(); p_list != gpio_list.end(); p_list++, i++)
    {
        bool io_flag = gpio&(0x1<<i);
        frame_map[*p_list] = to_string(io_flag);
    }
}
