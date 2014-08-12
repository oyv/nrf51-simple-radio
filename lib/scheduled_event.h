#ifndef SCHEDULED_EVENT_H_
#define SCHEDULED_EVENT_H_

#include <stdint.h>

typedef uint32_t schedule_time_t;

typedef struct scheduled_event_t
{
    uint32_t * task;
    schedule_time_t time;
} scheduled_event_t;


uint32_t scheduled_events_init();

uint32_t scheduled_events_schedule(uint32_t * task, schedule_time_t time);

#endif