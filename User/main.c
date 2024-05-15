#include "stm32f10x.h" // Device header
#include "delay.h"
#include "led.h"
#include "Key.h"
#include "usart.h"
#include "timer.h"
#include "adc.h"
#include "myiic.h"
#include "24cxx.h"
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_exti.h>
#include <stm32f10x_tim.h>

LoopTypedef Loop;
uint8_t KeyNum;
uint8_t Key[5];
uint16_t LED[8] = {0, 0, 38, 0, 0, 38, 0, 0};
int LED_i, Sound_i, Sound_e;
int hour = 11, min = 12, sec = 55;
int S_hour = 11, S_min = 13;
int Key_Flag, Key1_Flag, Key2_Flag, Key2_Flag, Key4_Flag, Time_Flag;
float adcx = 0.0;
float cputemp = 0.0;
uint16_t adcValue = 0;

void Loop_Check(void);
void Duty_Loop(void);

void Duty_1ms(void);
void Duty_2ms(void);
void Duty_5ms(void);
void Duty_10ms(void);
void Duty_20ms(void);
void Duty_50ms(void);
void Duty_100ms(void);
void Duty_1000ms(void);
void LED_Scan();
void play_music(void);

int main(void)
{
    uart_init(72, 9600);
    LED_Init(0);
    Key_Init();
    delay_init();
    TimerxInit(10, 7200);
    TemperatureAndLightAdcInit();
    AT24CXX_Init();
    S_hour = AT24CXX_ReadOneByte(41);
    S_min = AT24CXX_ReadOneByte(42);

    while (1)
    {
        Duty_Loop();
    }
}

void Loop_Check(void)
{
    Loop.Cnt_2ms++;
    Loop.Cnt_5ms++;
    Loop.Cnt_10ms++;
    Loop.Cnt_20ms++;
    Loop.Cnt_50ms++;
    Loop.Cnt_100ms++;
    Loop.Cnt_1000ms++;

    if (Loop.check_flag == 1)
    {
        Loop.err_flag++;
    }
    else
    {
        Loop.check_flag = 1;
    }
}

uint8_t get_buf(uint8_t buf)
{
    if (buf >= 'a')
    {
        return buf - 'a' + 10;
    }
    else if (buf >= 'A')
    {
        return buf - 'A' + 10;
    }
    else
    {
        return buf - '0';
    }
}

/**
 * @name   Duty_Loop()
 * @brief  主体框架循环
 * @param  NONE
 * @retval NONE
 */
void Duty_Loop(void)
{
    if (Loop.check_flag == 1)
    {
        Duty_1ms();

        if (Loop.Cnt_2ms >= 2)
        {
            Duty_2ms();
            Loop.Cnt_2ms = 0;
        }

        if (Loop.Cnt_5ms >= 5)
        {
            Duty_5ms();
            Loop.Cnt_5ms = 0;
        }

        if (Loop.Cnt_10ms >= 10)
        {
            Duty_10ms();
            Loop.Cnt_10ms = 0;
        }

        if (Loop.Cnt_20ms >= 20)
        {
            Duty_20ms();
            Loop.Cnt_20ms = 0;
        }

        if (Loop.Cnt_50ms >= 50)
        {
            Duty_50ms();
            Loop.Cnt_50ms = 0;
        }

        if (Loop.Cnt_100ms >= 100)
        {
            Duty_100ms();
            Loop.Cnt_100ms = 0;
        }

        if (Loop.Cnt_1000ms >= 1000)
        {
            Duty_1000ms();
            Loop.Cnt_1000ms = 0;
        }

        Loop.check_flag = 0;
    }
}

void Duty_1ms(void)
{
    LED_Scan();
    if (Key2_Flag == 0)
    {
        if (S_hour == hour && S_min == min)
            play_music();
    }
}

void Duty_2ms(void)
{
}

void Duty_5ms(void)
{
    if (USART_RX_STA & 0x80)
    {
        USART_RX_STA = 0;
    }
}

void Duty_10ms(void)
{
}

void Duty_20ms(void)
{
}
void Duty_50ms(void)
{
    if (Key4_Flag == 0)
    {
        LED[0] = hour / 10;
        LED[1] = hour % 10;
        LED[2] = 38;
        LED[3] = min / 10;
        LED[4] = min % 10;
        LED[5] = 38;
        LED[6] = sec / 10;
        LED[7] = sec % 10;
    }
}

void Duty_100ms()
{
    adcx = GetTemperature();       // 使用PC5 ADC1, 通道15
    cputemp = GetCPUTemperature(); // 使用内部CPU湿度ADC1, 通道16
    adcValue = GetLightAdc(14);    // 使用PC4 ADC1, 通道14

    if (Key4_Flag != 0)
    {
        LED[0] = adcx / 10;
        LED[1] = (u8)adcx % 10;
        LED[2] = (u8)(adcx * 10) % 10;
        LED[3] = 39;
        LED[4] = adcValue / 1000;
        LED[5] = adcValue % 1000 / 100;
        LED[6] = adcValue % 100 / 10;
        LED[7] = adcValue % 10;
    }
}

void Duty_1000ms(void)
{
    sec++;
    if (sec >= 60)
    {
        sec = 0;
        min++;
        if (min >= 60)
        {
            min = 0;
            hour++;
            if (hour >= 24)
                hour = 0;
        }
    }
}

void LED_Scan()
{
    if (Key4_Flag != 0 && LED_i == 1)
        PortationDisplay(LED_i, LED[LED_i]);
    else
        SetLed(LED_i, LED[LED_i]);
    LED_i++;
    LED_i %= 8;
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // 溢出中断
    {
        Loop_Check();
    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除中断标志位
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) == SET) // 判断是否是外部中断0号线触发的中断
    {
        /*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) == 0)
        {
            Key_Flag = 1;
            if (Time_Flag != 0)
            {
                if (min == 0)
                {
                    min = 59;
                    hour--;
                    if (hour < 0)
                        hour = 23;
                }
                else
                    min--;
            }
        }

        EXTI_ClearITPendingBit(EXTI_Line0); // 清除外部中断0号线的中断标志位
                                            // 中断标志位必须清除
                                            // 否则中断将连续不断地触发，导致主程序卡死
    }
}

void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) == SET) // 判断是否是外部中断0号线触发的中断
    {
        /*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0)
        {
            Key2_Flag = !Key2_Flag;
            if (Key2_Flag != 0)
                printf("已关闭闹钟 \r\n");
            if (Key2_Flag == 0)
                printf("已开启闹钟 \r\n");
        }

        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) == SET) // 判断是否是外部中断0号线触发的中断
    {
        /*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == 0)
        {
            Key_Flag = 3;
            if (Time_Flag != 0)
            {
                min++;
                if (min >= 60)
                {
                    min = 0;
                    hour++;
                    if (hour >= 24)
                        hour = 0;
                }
            }
        }

        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) == SET) // 判断是否是外部中断0号线触发的中断
    {
        /*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0)
        {
            Key4_Flag = !Key4_Flag;
            printf("当前温度：%3.2f℃，CPU温度：%3.2f℃，光照度：%4d \r\n", adcx, cputemp, adcValue);
        }

        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void Sound(u16 frq)
{
    u32 time;
    if (frq != 1000)
    {
        time = 100000 / ((u32)frq);
        GPIO_ResetBits(GPIOE, GPIO_Pin_5); // 打开蜂鸣器--根据自己的硬件情况调整，通常就是控制蜂鸣器的gpio引脚置1
        delay_us(time);
        GPIO_SetBits(GPIOE, GPIO_Pin_5); // 关闭蜂鸣器--根据自己的硬件情况调整，通常就是控制蜂鸣器的gpio引脚置0
        delay_us(time);
    }
    else
        delay_us(1000);
}

void play_music(void)
{
    //             低7  1   2   3   4   5   6   7  高1 高2  高3 高4 高5 不发音
    uc16 tone[] = {247, 262, 294, 330, 349, 392, 440, 494, 523, 587, 659, 698, 784, 1000}; // 音频数据表

    // 小燕子
    u8 music[] = {3, 5, 8, 6, 5, 13, // 音调
                  3, 5, 6, 8, 5, 13};
    u8 time[] = {1, 1, 1, 1, 3, 2, // 时间
                 1, 1, 1, 1, 3, 2};

    u32 yanshi;
    // u16 i, e;
    yanshi = 2; // 10;  4;  2

    if (Sound_e < ((u16)time[Sound_i]) * tone[music[Sound_i]] / yanshi)
        Sound((u32)tone[music[Sound_i]]);

    Sound_e++;
    if (Sound_e >= ((u16)time[Sound_i]) * tone[music[Sound_i]] / yanshi)
    {
        Sound_e = 0;
        Sound_i++;
        if (Sound_i >= sizeof(music) / sizeof(music[0]))
            Sound_i = 0;
    }

    // for (e = 0; e < ((u16)time[Sound_i]) * tone[music[Sound_i]] / yanshi; e++)
    // {
    //     Sound((u32)tone[music[Sound_i]]);
    // }
}

void USART1_IRQHandler(void)
{
    u8 res;
    if (USART1->SR & (1 << 5)) // 接收到数据
    {
        res = USART1->DR;
        // printf("%c", res); // 返回打印
        // if (0x0d == res)
        //     printf("\n");               // 回车的话加换行
        if (0x0d == res)
        {
            if (USART_RX_BUF[0] == 's' && USART_RX_BUF[1] == 'e' && USART_RX_BUF[2] == 't')
            {
                S_hour = get_buf(USART_RX_BUF[4]) * 10 + get_buf(USART_RX_BUF[5]);
                S_min = get_buf(USART_RX_BUF[6]) * 10 + get_buf(USART_RX_BUF[7]);
                AT24CXX_WriteOneByte(41,S_hour);
                AT24CXX_WriteOneByte(42,S_min);
                printf("闹钟设置为 %2d:%2d \r\n", S_hour, S_min);
            }
            else if (USART_RX_BUF[0] == 't' && USART_RX_BUF[1] == 'i' && USART_RX_BUF[2] == 'm' && USART_RX_BUF[3] == 'e')
            {
                Time_Flag = !Time_Flag;
                if (Time_Flag != 0)
                    printf("开始设置时间 \r\n");
                if (Time_Flag == 0)
                    printf("设置时间结束 \r\n");
            }
        }
        if ((USART_RX_STA & 0x80) == 0) // 接收未完成
        {
            if (USART_RX_STA & 0x40) // 接收到了0x0d
            {
                if (res != 0x0a)
                    USART_RX_STA = 0; // 接收错误,重新开始
                else
                    USART_RX_STA |= 0x80; // 接收完成了
            }
            else // 还没收到0X0D
            {
                if (res == 0x0d)
                {
                    USART_RX_BUF[USART_RX_STA & 0X3F] = '\0';
                    USART_RX_STA |= 0x40;
                }
                else
                {
                    USART_RX_BUF[USART_RX_STA & 0X3F] = res;
                    USART_RX_STA++;
                    if (USART_RX_STA > 63)
                        USART_RX_STA = 0; // 接收数据错误,重新开始接收
                }
            }
        }
    }
}
