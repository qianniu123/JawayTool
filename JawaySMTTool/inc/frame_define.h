#ifndef FRAME_DEFINE_H
#define FRAME_DEFINE_H

#include <stdint.h>

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
    char ccid[21];
    char csq;
    char gps_csq;
    char gps_snr;
    char ble_mac[6];
    char state;
    char model[2];//uint16
    char adc;//uint8
    char next[0];
}frame_msg_t;

typedef struct
{
   uint8_t acc;
   uint8_t externElec;
   uint8_t gSensor;
   uint8_t pwm;
   uint8_t lock;
   uint8_t muart;
   uint8_t adc;
   uint8_t rf;

}frame_state_t;



#pragma pack()



#ifdef __cplusplus
}
#endif

#endif // FRAME_DEFINE_H
