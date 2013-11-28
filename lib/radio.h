#ifndef RADIO_H_INCLUDED
#define RADIO_H_INCLUDED

#include <stdint.h>

#define RADIO_PACKET_MAX_LEN 64
#define RADIO_PACKET_BUFFER_SIZE 1

typedef enum
{
    PACKET_RECEIVED,
} radio_evt_type_t;

typedef struct
{
    uint8_t len;
    uint8_t data[RADIO_PACKET_MAX_LEN];
} radio_packet_t;

typedef struct
{
   radio_evt_type_t type;
   union
   {
       radio_packet_t packet;
   };
} radio_evt_t;

typedef void (radio_evt_handler_t)(radio_evt_t * evt);

uint32_t radio_init(radio_evt_handler_t * evt_handler);
uint32_t radio_send(radio_packet_t * packet);
uint32_t radio_receive_start(void);

#endif