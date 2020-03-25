#include "smttool.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <iostream>
#include <fstream>
#include "yaml-cpp/yaml.h"

#include <QDebug>

SMTTool::SMTTool(QObject *parent) : QObject(parent)
{
    //init
    loadMatnr();

}
SMTTool::~SMTTool()
{

}

void SMTTool::loadMatnr()
{
    //init model->MATNR->jaway config
#if 0
    model_matnr_config_t model_matnr_config[MATNR_NUM] =
     {
        {0x12, "30200-QSM-A510-4GEU", "B10_MB2", 2},
        {0x16, "30200-QSM-A510-NULL", "B10_MB8", 8},
        {0x13, "30200-TGC-A000-2GCHN", "B10_2G", 0},
        {0x1C, "30200-TGD-A000-2GCHN", "B10_2G", 0},
        {0x14, "30200-TBG-A000-NULL", "B10_MB7", 7},
        {0x15, "30200-TBG-A000-4GEU", "B10_MB1", 1},
        {0x1A, "30200-QSM-F000-4GEU", "B10_MB9", 9},
        {0x1D, "30200-QSM-F000-NULL", "B10_MB10", 9},
        {0x1E, "30200-QSM-F600-2GCHN", "B10_2G", 0},
        {0x19, "30200-TAC-A000-2GCHN", "B11_MB3", 12},
        {0x18, "30200-TAC-E300-4GEU", "B11_MB2", 11},
        {0x1F, "30200-TAC-E300-NULL", "B11_MB4", 11},
        {0x17, "30200-TAC-E601-NULL", "B11_MB1", 10},
        {0x1B, "30200-QSM-G500-4GEU", "B10_MB9", 9}
     };

    int size = sizeof(model_matnr_config)/sizeof(model_matnr_config_t);
    map<string, string> second;
    for(int i=0; i<size; i++)
    {
       model_matnr_table[i] = model_matnr_config[i];
       matnr_strlist << QString::fromStdString(model_matnr_config[i].matnr);

       second.clear();
       second[model_matnr_config[i].matnr] = model_matnr_config[i].config;
       model_matnr_map[model_matnr_config[i].model] = second;

       model_deviceType_map[model_matnr_config[i].model] = model_matnr_config[i].device_type;
    }
#endif
    //-----------------------------------------------------------------
    map<string, string> matnr_map; //must before goto label;

    ifstream f_in("./matnr.yml");
    if(!f_in)
    {
        qDebug() << "read matnr.yml error";
        return;
    }

    YAML::Node matnr_yml = YAML::Load(f_in);
    if(!matnr_yml.IsDefined())
    {
        qDebug() << "yaml load error";
        goto LOAD_END;
    }

    model_matnr_map.clear();
    model_deviceType_map.clear();

    for(YAML::const_iterator it=matnr_yml.begin(); it!=matnr_yml.end(); it++)
    {
        string key = it->first.as<string>();  //cout << "----------------key:" << key << endl;
        YAML::Node node = it->second.as<YAML::Node>();
        YAML::const_iterator it_node;
        for(it_node=node.begin(); it_node!=node.end(); it_node++)
        {
            string matnr = it_node->first.as<string>(); //cout << "matnr:" << matnr << "\t";
            matnr_strlist << QString::fromStdString(matnr);

            if(node[matnr]["config"])
            {
                string config = node[matnr]["config"].as<string>(); //cout << "config:" << config << "\t";
                matnr_map.clear();
                matnr_map[matnr] = config;
            }
            uint16_t model;
            if(node[matnr]["model"])
            {
                model = node[matnr]["model"].as<uint16_t>(); //cout << "model:" << model << "\t";
                model_matnr_map[model] = matnr_map;

            }
            if(node[matnr]["deviceType"])
            {
                uint16_t deviceType = node[matnr]["deviceType"].as<uint16_t>(); //cout << "deviceType:" << deviceType << endl;
                model_deviceType_map[model] = (uint8_t)deviceType;

                if(node[matnr]["deviceName"])
                {
                    string deviceName = node[matnr]["deviceName"].as<string>();
                    deviceType_deviceName_map[deviceType] = deviceName;
                }
            }

        }
    }

    LOAD_END:
    f_in.close();
}

void SMTTool::loadConfig()
{
    //config.yaml --> config_map --> tableWidget
    ifstream f_in("./config.yml");
    if(!f_in)
    {
        qDebug() << "read config.yml error";
        return;
    }

    YAML::Node config = YAML::Load(f_in);
    if(!config.IsDefined()) //if(config.IsNull())
    {
        qDebug() << "yaml load error";
        goto LOAD_END;
    }

    config_map.clear();
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
        config_map[node_name].enable = enable;
        config_map[node_name].type   = type;
        config_map[node_name].value  = value;
    }

    LOAD_END:
    f_in.close();
}

//config_map --> config.yml
void SMTTool::saveConfig()
{
    ofstream f_out("./config.yml");
    if(!f_out)
    {
        qDebug() << "open config.yml error";
    }
    YAML::Node config;

    //--->config.yml
    map<string, item_t>::iterator it;
    for(it=config_map.begin(); it!=config_map.end(); it++)
    {
        string node_name = it->first;
        item_t item = it->second;
        config[node_name]["enable"] = item.enable;
        config[node_name]["type"] = item.type;
        config[node_name]["value"] = item.value;
    }

    f_out << config;
    f_out.close();
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
    frame_map["DEVICE_TYPE"] = to_string(p_frame->deviceType); //系统(设备)类型
    frame_map["MOTOR_TYPE"] = to_string(p_frame->motorType); //车型

    //type = int-->string
    frame_map["GSM"] = to_string(p_frame->csq);
    frame_map["GPS"] = to_string(p_frame->gps_csq);
    frame_map["SNR"] = to_string(p_frame->gps_snr);
    frame_map["ADC"] = to_string(p_frame->adc);

    //type = short-->string
    frame_map["MODEL"] = to_string(*(uint16_t*)p_frame->model); //型号（料号）

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
