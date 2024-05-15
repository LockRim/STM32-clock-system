#include "timer.h"
#include <misc.h>
#include <stm32f10x_tim.h>

// 数字钟的时，分、秒
// u8 hour = 0, minute = 0, second = 0;

/****************通用定时器初始化函数********************
 * 通用定时器中断初始化
 * 这里时钟选择为APB1的2倍，而APB1为36M
 * arr：自动重装值。
 * psc：时钟预分频数
 * 这里使用的是定时器3!
 ******************************************************/
void TimerxInit(u16 arr, u16 psc)
{
    RCC->APB1ENR |= 1 << 1; // TIM3时钟使能
    TIM3->ARR = arr;        // 设定计数器自动重装值，10为1ms
    TIM3->PSC = psc;        // 预分频器7200，得到10KHZ的计数时钟

    TIM3->DIER |= 1 << 0; // 允许更新中断
    TIM3->CR1 |= 0x01;    // 使能定时器3

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;                      // 定义结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;           // 选择配置NVIC的USART1线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 指定NVIC线路使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 指定NVIC线路的抢占优先级为1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 指定NVIC线路的响应优先级为1
    NVIC_Init(&NVIC_InitStructure);                           // 将结构体变量交给NVIC_Init，配置NVIC外设
}

/****************定时器3的中断函数********************
 * 定时器3的中断函数
 **************************************************/
// void TIM3_IRQHandler(void)
// {
//     if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // 溢出中断
//     {
//         Loop_Check();
//     }
//     TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除中断标志位
// }

