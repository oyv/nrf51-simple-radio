#ifndef GRASSHOPPER_H_
#define GRASSHOPPER_H_

#include <stdint.h>
#include "radio.h"
#include "scheduled_events.h"

typedef struct
{
    radio_packet_t radio_packet;
} grasshopper_packet_t;

uint32_t grasshopper_init(radio_evt_handler_t * evt_handler);
void grasshopper_test(void);

uint32_t grasshopper_send_packet(grasshopper_packet_t * packet, schedule_time_t time);

#endif
