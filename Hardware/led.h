#ifndef __LED_H
#define __LED_H

#include <stm32f10x.h>

// 位选
#define SEL0 PBout(0)
#define SEL1 PBout(1)
#define SEL2 PBout(2)

void LED_Init(int i);
void SetLed(uint8_t w, uint8_t value);
void PortationDisplay(u8 w, u8 value); //数码管显示带小数点的数值

#endif
