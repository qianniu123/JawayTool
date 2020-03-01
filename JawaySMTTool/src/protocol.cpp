#include "protocol.h"

Protocol::Protocol()
{

}


void Protocol::parse_pack(char *pack, int data_len)//
{
    //get value
    frame_t *p_frame = (frame_t*)pack;

    //save value to key-value(string, string);
    frame_map.clear();
    item_t item;
    //----------------------------------
    item.type = "string";//type = string

    char imei[15+1] = {0};
    memcpy(imei, p_frame->imei, sizeof(p_frame->imei));
    item.value = string(imei);
    frame_map["IMEI"] = item;
    char sn[16+1] = {0};
    memcpy(sn, p_frame->sn, sizeof(p_frame->sn));
    item.value = string(sn);
    frame_map["SN"] = item;
    char softVer[8+1] = {0};
    memcpy(softVer, p_frame->softVer, sizeof(p_frame->softVer));
    item.value = string(softVer);
    frame_map["SOFTVER"] = item;
    char hardVer[8+1] = {0};
    memcpy(hardVer, p_frame->hardVer, sizeof(p_frame->hardVer));
    item.value = string(hardVer);
    frame_map["HARDVER"] = item;
    char iccid[21+1] = {0};
    memcpy(iccid, p_frame->iccid, sizeof(p_frame->iccid));
    item.value = string(iccid);
    frame_map["ICCID"] = item;
    char ble_mac[6+1] = {0};
    memcpy(ble_mac, p_frame->ble_mac, sizeof(p_frame->ble_mac));
    item.value = string(ble_mac);
    frame_map["BLE_MAC"] = item;

    //---------------------------------------------
    item.type = "int";//type = enum-->int-->string

    item.value = to_string(p_frame->deviceType);
    frame_map["DEVICE_TYPE"] = item;
    item.value = to_string(p_frame->motorType);
    frame_map["MOTOR_TYPE"] = item;
    //type = int-->string
    item.value = to_string(p_frame->csq);
    frame_map["GSM"] = item;
    item.value = to_string(p_frame->gps_csq);
    frame_map["GPS"] = item;
    item.value = to_string(p_frame->gps_snr);
    frame_map["SNR"] = item;
    item.value = to_string(p_frame->adc);
    frame_map["ADC"] = item;
    //type = short-->string
    item.value = to_string(*p_frame->model);
    frame_map["MODEL"] = item;

    //state----------------------------------
    char state_flag = p_frame->state_flag;
    //---------------------------------------
    item.type = "bool";//type = bool-->string

    bool acc_flag = state_flag&0x01;
    item.value = to_string(acc_flag);
    frame_map["电门"] = item;
    bool vin_flag = (state_flag&0x02) >> 1;
    item.value = to_string(vin_flag);
    frame_map["外电"] = item;
    bool Gsensor_flag = (state_flag&0x04) >> 2;
    item.value = to_string(Gsensor_flag);
    frame_map["Gsensor"] = item;
    bool pwm_flag = (state_flag&0x08) >> 3;
    item.value = to_string(pwm_flag);
    frame_map["PWM"] = item;
    bool lock_flag = (state_flag&0x10) >> 4;
    item.value = to_string(lock_flag);
    frame_map["LOCK"] = item;
    bool muart_flag = (state_flag&0x20) >> 5;
    item.value = to_string(muart_flag);
    frame_map["MUART"] = item;
    bool adc_flag = (state_flag&0x40) >> 6;
    item.value = to_string(adc_flag);
    frame_map["ADC_FLAG"] = item;
    bool rf_flag = (state_flag&0x80) >> 7;
    item.value = to_string(rf_flag);
    frame_map["RF"] = item;

    //gpio---------------------------------------------------------------
    //printf("data size = %d, sizeof(frame_t)=%d\n", size, sizeof(frame_t));
    //type = bool-->string
    if(data_len < (int)sizeof(frame_t)+4) //sizeof(frame_t) = 83;
    {
        return;
    }
    char gpio_c[4]; //4 byte gpio value
    memcpy(gpio_c, p_frame->next, sizeof(gpio_c));
    int gpio_i = *(int*)gpio_c;
    //p_frame->next=gpio_c= 7F FF 1F 00; --> gpio_i=0x001FFF7F;
    //-----------------------------------------
    item.type = "io";//type = io; bool-->string

    vector<string>::iterator iter;
    int i=0;
    for(iter=gpios.begin(); iter!=gpios.end(); iter++, i++)
    {
        bool io_flag = gpio_i&(0x1<<i);
        item.value = to_string(io_flag);
        frame_map[*iter] = item;
    }

}
