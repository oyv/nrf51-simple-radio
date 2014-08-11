#include <stdio.h>

#include "nrf51.h"
#include "nrf_delay.h"

#include "error.h"
#include "gpio.h"
#include "leds.h"
#include "radio.h"

volatile uint32_t n_packets_received = 0;

uint8_t dev_addr[5] = {0x01, 0x23, 0x45, 0x67, 0x89};
uint8_t broadcast_addr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
uint8_t tx_addr[5] = {0x01, 0x23, 0x45, 0x67, 0x89};

void error_handler(uint32_t err_code, uint32_t line_num, char * file_name)
{
    volatile uint32_t m_err_code = err_code;
    volatile uint32_t m_line_num = line_num;
    char * volatile m_file_name  = file_name;
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
        case PACKET_RECEIVED:
            led_toggle(LED1);
            n_packets_received++;
            break;

        default:
            break;
    }
}

int main(void)
{
    static uint32_t err_code;
    err_code = radio_init(radio_evt_handler, broadcast_addr, dev_addr);
    ASSUME_SUCCESS(err_code);
    
    radio_set_tx_address(tx_addr);

    leds_init();

    while (1)
    {
        err_code = radio_start_rx();
        led_on(LED0);
        nrf_delay_us(50000);
        led_off(LED0);
        nrf_delay_us(100000);
    }
}
