#ifndef FRAME_DEFINE_H
#define FRAME_DEFINE_H

#include <stdint.h>

//=========================================
#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(0x1)

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

//gpio[4]

#pragma pack()

#ifdef __cplusplus
}
#endif
//=======================================
#include <map>
#include <string>
using namespace std;
/*
typedef struct
{
    map<string, string> imei;
    map<string, string> sn;
    map<string, string> software;
    map<string, string> hardware;
    map<string, char>   deviceType;
    map<string, char>   motorType;
    map<string, string> iccid;
    map<string, char>   csq;
    map<string, char>   gps_csq;
    map<string, char>   gps_snr;
    map<string, string> ble_mac;
    map<string, char>   state_flag;
    map<string, string> model;
    map<string, char>   adc;
    //----------------------------
    map<string, string> gpio_flag;
}frame_map_t;
*/
/*
typedef enum
{
    STRING,
    BOOL,
    INT,
}type_t;

typedef struct
{
    type_t type;

}item_t;
*/




#endif // FRAME_DEFINE_H
