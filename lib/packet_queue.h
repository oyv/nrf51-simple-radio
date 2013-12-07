#ifndef PACKET_QUEUE_H_INCLUDED__
#define PACKET_QUEUE_H_INCLUDED__

#include "radio.h"

#include <stdbool.h>

#define PACKET_QUEUE_SIZE 10

typedef struct
{
    radio_packet_t * head;
    radio_packet_t * tail;
    radio_packet_t packets[PACKET_QUEUE_SIZE];
} packet_queue_t;

uint32_t packet_queue_init(packet_queue_t * queue);

bool packet_queue_is_empty(packet_queue_t * queue);

bool packet_queue_is_full(packet_queue_t * queue);

uint32_t packet_queue_add(packet_queue_t * queue, radio_packet_t * packet);

uint32_t packet_queue_get(packet_queue_t * queue, radio_packet_t * packet);

#endif