#ifndef PROTOCOL_H
#define PROTOCOL_H
/*
    this class is about protocal and config.yaml file
*/

#include <stdint.h>

#pragma pack(0x1)
//protocol
typedef struct
{
    char imei[15];
    char sn[16];
    char softVer[8];
    char hardVer[8];
    char deviceType;
    char motorType;
    char iccid[21];
    char csq;
    char gps_csq;
    char gps_snr;
    char ble_mac[6];
    char state_flag;
    char model[2];//uint16
    char adc;//uint8
    char next[0];
}frame_t;

typedef struct
{
   uint8_t acc;
   uint8_t vin;
   uint8_t gSensor;
   uint8_t pwm;
   uint8_t lock;
   uint8_t muart;
   uint8_t adc;
   uint8_t rf;
}state_flag_t;

#pragma pack()

//--------------------------------------------
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

using namespace std;

typedef struct
{
    string enable;
    string type;
    string value;
}item_t;

class Protocol
{
public:
    Protocol();

//model: data
    unordered_map<string, item_t> frame_map;
    unordered_map<string, item_t> home_map;
    unordered_map<string, item_t> config_map;
    vector<string> gpios;

//data process
    void parse_pack(char *pack, int size);

    void save_log();
};

#endif // PROTOCOL_H
