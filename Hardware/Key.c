#include "stm32f10x.h" // Device header
#include "Delay.h"
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_exti.h>
#include <misc.h>

extern uint8_t KeyNum; // 定义变量，默认键码值为0
int keyn = 0;

/**
 * 函    数：按键初始化
 * 参    数：无
 * 返 回 值：无
 */
void Key_Init(void)
{
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // 开启GPIOC的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /*GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure); // 将PC0~PC3引脚初始化为上拉输入

    /*AFIO选择中断引脚*/
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource0); // 将外部中断的0号线映射到GPIOC，即选择PC0为外部中断引脚
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1); // 将外部中断的1号线映射到GPIOC，即选择PC1为外部中断引脚
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2); // 将外部中断的2号线映射到GPIOC，即选择PC2为外部中断引脚
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource3); // 将外部中断的3号线映射到GPIOC，即选择PC3为外部中断引脚

    /*EXTI初始化*/
    EXTI_InitTypeDef EXTI_InitStructure;                                              // 定义结构体变量
    EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3; // 选择配置外部中断的0号线和1号线
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;                                         // 指定外部中断线使能
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;                               // 指定外部中断线为中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;                           // 指定外部中断线为上升沿触发
    EXTI_Init(&EXTI_InitStructure);                                                   // 将结构体变量交给EXTI_Init，配置EXTI外设

    /*NVIC中断分组*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 配置NVIC为分组2
                                                    // 即抢占优先级范围：0~3，响应优先级范围：0~3
                                                    // 此分组配置在整个工程中仅需调用一次
                                                    // 若有多个中断，可以把此代码放在main函数内，while循环之前
                                                    // 若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置

    /*NVIC配置*/
    NVIC_InitTypeDef NVIC_InitStructure;                      // 定义结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;          // 选择配置NVIC的EXTI0线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 指定NVIC线路使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 指定NVIC线路的抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 指定NVIC线路的响应优先级为1
    NVIC_Init(&NVIC_InitStructure);                           // 将结构体变量交给NVIC_Init，配置NVIC外设

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;          // 选择配置NVIC的EXTI1线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 指定NVIC线路使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 指定NVIC线路的抢占优先级为1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 指定NVIC线路的响应优先级为1
    NVIC_Init(&NVIC_InitStructure);                           // 将结构体变量交给NVIC_Init，配置NVIC外设

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;          // 选择配置NVIC的EXTI2线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 指定NVIC线路使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 指定NVIC线路的抢占优先级为2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 指定NVIC线路的响应优先级为1
    NVIC_Init(&NVIC_InitStructure);                           // 将结构体变量交给NVIC_Init，配置NVIC外设

    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;          // 选择配置NVIC的EXTI3线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 指定NVIC线路使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 指定NVIC线路的抢占优先级为3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 指定NVIC线路的响应优先级为1
    NVIC_Init(&NVIC_InitStructure);                           // 将结构体变量交给NVIC_Init，配置NVIC外设
}

uint8_t Key_GetNum(void)
{
    uint8_t KeyNum = 0; // 定义变量，默认键码值为0

    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) == 0) // 读输入寄存器的状态，如果为0，则代表按键1按下
    {
        keyn++;
        if (keyn >= 10)
        {
            while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) == 0)
                ;       // 等待按键松手
            KeyNum = 1; // 置键码为1
            keyn = 0;
        }
    }

    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0) // 读输入寄存器的状态，如果为0，则代表按键2按下
    {
        keyn++;
        if (keyn >= 100)
        {
            while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0)
                ;       // 等待按键松手
            KeyNum = 2; // 置键码为2
            keyn = 0;
        }
    }

    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == 0) // 读输入寄存器的状态，如果为0，则代表按键2按下
    {
        // while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == 0);       // 等待按键松手
        KeyNum = 3; // 置键码为3
    }

    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0) // 读输入寄存器的状态，如果为0，则代表按键2按下
    {
        keyn++;
        if (keyn >= 100)
        {
            while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0)
                ;       // 等待按键松手
            KeyNum = 4; // 置键码为4
            keyn = 0;
        }
    }

    return KeyNum; // 返回键码值，如果没有按键按下，所有if都不成立，则键码为默认值0
}

void Key_SetNum(uint8_t Num)
{
    KeyNum = Num;
}
