#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "nrf51.h"
#include "nrf_delay.h"

#include "error.h"
#include "gpio.h"
#include "leds.h"
#include "grasshopper.h"
#include "scheduled_events.h"

volatile uint32_t n_packets_sent = 0;
volatile uint32_t n_packets_lost = 0;
volatile uint32_t n_packets_received = 0;

volatile uint32_t test_counter = 0;

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
static void radio_evt_handler(radio_evt_t * evt)
{
    switch (evt->type)
    {
        case PACKET_SENT:
            led_toggle(LED1);
            n_packets_sent++;
            break;

        case PACKET_RECEIVED:
            led_toggle(LED0);
            n_packets_received++;
            break;

        default:
            break;
    }

}

int main(void)
{
    uint8_t i = 0; 
    uint32_t err_code;
    schedule_time_t send_time, current_time;
    
    send_time.timer_tick = 0;

    leds_init();

    grasshopper_packet_t packet;
    packet.radio_packet.len = 4;

    grasshopper_init(radio_evt_handler);
    
    grasshopper_test();
    
    while (1)
    {
        test_counter++;
        scheduled_events_get_current_time(&current_time);
        send_time.rtc_tick = current_time.rtc_tick + 20000;
        packet.radio_packet.data[0] = i++;
        packet.radio_packet.data[1] = 0x12;

        err_code = grasshopper_send_packet(&packet, send_time);
        ASSUME_SUCCESS(err_code);

        nrf_delay_us(2000);

        radio_stop_rx();

        nrf_delay_us(1000000);
    }
}
