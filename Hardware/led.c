#include "stm32f10x.h"
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "led.h"

u8 segTable[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // B
    0x39, // C
    0x5E, // D
    0x79, // E
    0x71, // F
    0x3D, // G
    0x76, // H
    0x0F, // I
    0x0E, // J
    0x75, // K
    0x38, // L
    0x37, // M
    0x54, // N
    0x5C, // O
    0x73, // P
    0x67, // Q
    0x31, // R
    0x49, // S
    0x78, // T
    0x3E, // U
    0x1C, // V
    0x7E, // W
    0x64, // X
    0x6E, // Y
    0x5A, // Z
    0x08, // _
    0x80, // .
    0x40, // -
    0x00, // 空
};

u8 segTablePortation[] = {0xbf, 0x86, 0xdb, 0xcf, 0xe6, 0xed, 0xfd, 0x87, 0xff, 0xef};

// i==0,led灯，为1时是数码管
void LED_Init(int i)
{
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  // 开启AFIO的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 开启GPIOB的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); // 开启GPIOE的时钟
                                                          // 使用各个外设前必须开启时钟，否则对外设的操作无效

    // 只关闭JTAG而保留SWD
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    /*GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStructure; // 定义结构体变量

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // GPIO模式，赋值为推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速度，赋值为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);            // PB4推挽输出
                                                      // 将赋值后的构体变量传递给GPIO_Init函数
                                                      // 函数内部会自动根据结构体的参数配置相应寄存器
                                                      // 实现GPIOE的初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_WriteBit(GPIOB, GPIO_Pin_4, (BitAction)i);
}

void LedValue(uint8_t value) //
{
    // GPIOE->BRR = 0xff << 8;
    GPIO_ResetBits(GPIOE, 0xff << 8);
    // GPIOE->BSRR = value << 8;
    GPIO_SetBits(GPIOE, value << 8);
}

void SetLed(uint8_t w, uint8_t value)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, (BitAction)(w % 2));
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, (BitAction)(w / 2 % 2));
    GPIO_WriteBit(GPIOB, GPIO_Pin_2, (BitAction)(w / 4));
    LedValue(segTable[value]);
}

void SetLedPoint(uint8_t w, uint8_t value)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, (BitAction)(w % 2));
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, (BitAction)(w / 2 % 2));
    GPIO_WriteBit(GPIOB, GPIO_Pin_2, (BitAction)(w / 4));
    LedValue(segTable[value] | 0x80);
}

/***************************************
 * 数码管显示带小数点的数值
 * 参数 w:显示的位置，即位选，左-右：0-7
 *      value:要显示的数值
 ****************************************/
void PortationDisplay(u8 w, u8 value)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, (BitAction)(w % 2));
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, (BitAction)(w / 2 % 2));
    GPIO_WriteBit(GPIOB, GPIO_Pin_2, (BitAction)(w / 4));
    LedValue(segTablePortation[value]);
}
