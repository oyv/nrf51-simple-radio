#include "grasshopper.h"

#include <stdint.h>

#include "nrf51.h"
#include "nrf51_bitfields.h"

#include "radio.h"
#include "scheduled_events.h"
#include "error.h"

uint8_t dev_addr[5] = {0x01, 0x23, 0x45, 0x67, 0x89};
uint8_t broadcast_addr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
uint8_t tx_addr[5] = {0x01, 0x23, 0x45, 0x67, 0x89};

radio_evt_handler_t * m_evt_handler;

void radio_evt_handler(radio_evt_t * evt);


uint32_t grasshopper_init(radio_evt_handler_t * evt_handler)
{
    uint32_t err_code;
    
    m_evt_handler = evt_handler;
    err_code = radio_init(radio_evt_handler, broadcast_addr, dev_addr);
    if (err_code == SUCCESS)
        scheduled_events_init();
    
    radio_set_tx_address(tx_addr);
    
    
    return err_code;
}



void send_callback()
{
    uint32_t err_code = radio_prepare_send();
    ASSUME_SUCCESS(err_code);
}

void send_stop_rx_callback()
{
    uint32_t err_code = radio_stop_rx();
    ASSUME_SUCCESS(err_code);
}


void radio_evt_handler(radio_evt_t * evt)
{
    switch (evt->type)
    {
        case TRANSFER_EVENT_DONE:
            // TODO: scheduled_events_cancel(xxxx);
            break;
            
        case PACKET_SENT:
            break;

        case PACKET_RECEIVED:
            break;
        
        default:
            break;
    }
    (*m_evt_handler)(evt);
}

uint32_t grasshopper_send_packet(grasshopper_packet_t * packet, schedule_time_t time)
{
    scheduled_event_t send_event_clock_start =
    {
        .task = &NRF_CLOCK->TASKS_HFCLKSTART,
        .time = 
        {
            .rtc_tick = time.rtc_tick - 50,
            .timer_tick = time.timer_tick,
        },
        .callback = 0,
    };
    
    scheduled_event_t send_event_radio_enable = 
    {
        .task = &NRF_RADIO->TASKS_TXEN,
        .time = 
        {
            .rtc_tick = (time.rtc_tick - 5),
            .timer_tick = time.timer_tick,
        },
        .callback = send_callback,
    };
    
    
    scheduled_event_t send_event_start_tx = 
    {
        .task = &NRF_RADIO->TASKS_START,
        .time = time,
        .callback = 0,
    };
    
    scheduled_event_t send_event_stop_rx = 
    {
        .task = 0,
        .time = 
        {
            .rtc_tick = (time.rtc_tick + 50),
            .timer_tick = time.timer_tick,
        },
        .callback = send_callback,
    };
    
    
    uint32_t err_code;
    
    err_code = scheduled_events_schedule(&send_event_clock_start);
    if (err_code == SUCCESS)
        err_code = scheduled_events_schedule(&send_event_radio_enable);
    if (err_code == SUCCESS)
        err_code = scheduled_events_schedule(&send_event_start_tx);
    if (err_code == SUCCESS)
        err_code = scheduled_events_schedule(&send_event_stop_rx);
    
    return err_code;
}


