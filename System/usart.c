#include "sys.h"
#include "usart.h"
#include "led.h"
#include "delay.h"
#include <misc.h>
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
//********************************************************************************
// V1.3
// 支持适应不同频率下的串口波特率设置.
// 加入了对printf的支持
// 增加了串口接收命令功能.
// 修正了printf第一个字符丢失的bug
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// 加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
// 标准库需要的支持函数
struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};
/* FILE is typedef’ d in stdio.h. */
FILE __stdout;
// 定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}
// 重定义fputc函数
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0)
        ; // 循环发送,直到发送完毕
    USART1->DR = (u8)ch;
    return ch;
}
#endif
// end
//////////////////////////////////////////////////////////////////

#ifdef EN_USART1_RX // 如果使能了接收
// 串口1中断服务程序
// 注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF[64]; // 接收缓冲,最大64个字节.
// 接收状态
// bit7，接收完成标志
// bit6，接收到0x0d
// bit5~0，接收到的有效字节数目
u8 USART_RX_STA = 0; // 接收状态标记

#endif
// 初始化IO 串口1
// pclk2:PCLK2时钟频率(Mhz)
// bound:波特率
// CHECK OK
// 091209
void uart_init(u32 pclk2, u32 bound)
{
    float temp;
    u16 mantissa;
    u16 fraction;
    temp = (float)(pclk2 * 1000000) / (bound * 16); // 得到USARTDIV
    mantissa = temp;                                // 得到整数部分
    fraction = (temp - mantissa) * 16;              // 得到小数部分
    mantissa <<= 4;
    mantissa += fraction;
    RCC->APB2ENR |= 1 << 2;  // 使能PORTA口时钟
    RCC->APB2ENR |= 1 << 14; // 使能串口时钟
    GPIOA->CRH &= 0XFFFFF00F;
    GPIOA->CRH |= 0X000008B0; // IO状态设置

    RCC->APB2RSTR |= 1 << 14;    // 复位串口1
    RCC->APB2RSTR &= ~(1 << 14); // 停止复位
    // 波特率设置
    USART1->BRR = mantissa; // 波特率设置
    USART1->CR1 |= 0X200C;  // 1位停止,无校验位.
#ifdef EN_USART1_RX         // 如果使能了接收
    // 使能接收中断
    USART1->CR1 |= 1 << 8; // PE中断使能
    USART1->CR1 |= 1 << 5; // 接收缓冲区非空中断使能
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;                      // 定义结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;         // 选择配置NVIC的USART1线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 指定NVIC线路使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 指定NVIC线路的抢占优先级为1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 指定NVIC线路的响应优先级为1
    NVIC_Init(&NVIC_InitStructure);                           // 将结构体变量交给NVIC_Init，配置NVIC外设
#endif
}
