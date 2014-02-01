#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "nrf51.h"
#include "nrf_delay.h"

#include "error.h"
#include "gpio.h"
#include "leds.h"
#include "buttons.h"
#include "radio.h"

static radio_packet_t packet = 
{
    .len = 3,
    .flags = 0
};


void error_handler(uint32_t err_code, uint32_t line_num, char * file_name)
{
    volatile uint32_t err_code_copy = err_code;
    while (1)
    {
        for (uint8_t i = LED_START; i < LED_STOP; i++)
        {
            led_toggle(i);
            nrf_delay_us(50000);
        }
    }
}
void radio_evt_handler(radio_evt_t * evt)
{
    switch (evt->type)
    {
        case PACKET_SENT:
            //led_toggle(LED1);
            //radio_receive_start();
            led_on(LED1);
            nrf_delay_us(10000);
            led_off(LED1);
            nrf_delay_us(10000);
            led_on(LED1);
            nrf_delay_us(10000);
            led_off(LED1);
            break;

        case PACKET_LOST:
            //led_toggle(LED0);
            //radio_receive_start();
            led_on(LED1);
            nrf_delay_us(10000);
            led_off(LED1);
            break;
                
        case PACKET_RECEIVED:
            if (evt->packet.len > 2)
            {
                switch((radio_evt_type_t)evt->packet.data[0])
                {
                case BUTTON0_ON:
                    led_on(LED1);
                    break;
                case BUTTON0_OFF:
                    led_off(LED1);
                    break;
                case BUTTON1_ON:
                    led_on(LED1);
                    break;
                case BUTTON1_OFF:
                    led_off(LED1);
                    break;
                default:
                    break;
                }
            }
            break;
        
        case BUTTON0_ON:
        case BUTTON0_OFF:
        case BUTTON1_ON:
        case BUTTON1_OFF:
            packet.data[0] = (uint8_t)evt->type;
            radio_send(&packet);
            break;
        default:
            break;
    }

}

int main(void)
{
    //uint32_t err_code;

    leds_init();
    radio_init(radio_evt_handler);
    buttons_init();

    radio_receive_start();
    while (1)
    {
        nrf_delay_us(990000);
        led_toggle(LED0);
        nrf_delay_us(10000);
        led_toggle(LED0);
    }
}
