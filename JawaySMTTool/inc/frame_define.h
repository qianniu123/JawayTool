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

/*//物料号(MATNR)-----> 嘉为配置(config)-----> 型号(model)
30200-QSM-A510-4GEU     B10_MB2     0x12
30200-QSM-A510-NULL     B10_MB8     0x16

30200-TGC-A000-2GCHN    B10_2G      0x13
30200-TGD-A000-2GCHN    B10_2G      0x1C

30200-TBG-A000-NULL     B10_MB7     0x14
30200-TBG-A000-4GEU     B10_MB1     0x15

30200-QSM-F000-4GEU     B10_MB9     0x1A
30200-QSM-F000-NULL     B10_MB10    0x1D
30200-QSM-F600-2GCHN	B10_2G      0x1E

30200-TAC-A000-2GCHN	B11_MB3     0x19
30200-TAC-E300-4GEU     B11_MB2     0x18
30200-TAC-E300-NULL     B11_MB4     0x1F
30200-TAC-E601-NULL     B11_MB1     0x17

30200-QSM-G500-4GEU     B10_MB9     0x1B
*/

#define MATNR_NUM 14
typedef struct
{
    uint16_t model;
    string   matnr;
    string   config;
    uint8_t  device_type;
}model_matnr_config_t;


#endif // FRAME_DEFINE_H
