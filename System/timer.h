/****************定时器********************
 *
 *******************************************/

#ifndef _TIMER_H
#define _TIMER_H

#include "sys.h"
#include "delay.h"
#include "led.h"

typedef struct
{
    unsigned short int Cnt_1ms;
    unsigned short int Cnt_2ms;
    unsigned short int Cnt_5ms;
    unsigned short int Cnt_10ms;
    unsigned short int Cnt_20ms;
    unsigned short int Cnt_50ms;
    unsigned short int Cnt_100ms;
    unsigned short int Cnt_1000ms;

    unsigned char check_flag;
    unsigned char err_flag;
} LoopTypedef;

void TimerxInit(u16 arr, u16 psc); // 定时器初始化

#endif
