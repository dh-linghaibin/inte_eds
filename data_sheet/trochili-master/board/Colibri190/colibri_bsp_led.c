/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "gd32f1x0.h"
#include "colibri_bsp_led.h"

/*************************************************************************************************
 *  ���ܣ���ʼ���û�Led�豸                                                                      *
 *  ��������                                                                                     *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void EvbLedConfig(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9 |
                            GPIO_PIN_10);
}



/*************************************************************************************************
 *  ���ܣ�����Led�ĵ�����Ϩ��                                                                    *
 *  ������(1) index Led�Ʊ��                                                                    *
 *        (2) cmd   Led�Ƶ�������Ϩ�������                                                      *
 *  ���أ���                                                                                     *
 *  ˵����                                                                                       *
 *************************************************************************************************/
void EvbLedControl(int index, int cmd)
{
    switch (index)
    {
        case LED1:
        {
            if (cmd == LED_ON)
            {
                gpio_bit_set(GPIOB, GPIO_PIN_8); /*����Led1��*/
            }
            else
            {
                gpio_bit_reset(GPIOB, GPIO_PIN_8); /*Ϩ��Led1��*/
            }
            break;
        }
        case LED2:
        {
            if (cmd == LED_ON)
            {
                gpio_bit_set(GPIOB, GPIO_PIN_9); /*����Led2��*/
            }
            else
            {
                gpio_bit_reset(GPIOB, GPIO_PIN_9); /*Ϩ��Led2��*/
            }
            break;
        }
        case LED3:
        {
            if (cmd == LED_ON)
            {
                gpio_bit_set(GPIOB, GPIO_PIN_10); /*����Led3��*/
            }
            else
            {
                gpio_bit_reset(GPIOB, GPIO_PIN_10); /*Ϩ��Led3��*/
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
