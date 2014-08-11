#include "packet.h"
#include "error.h"


bool packet_is_empty(radio_packet_t* packet)
{
    return (packet->len == 0);
}
