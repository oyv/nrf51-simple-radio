#ifndef SCHEDULED_EVENT_H_
#define SCHEDULED_EVENT_H_

#include <stdint.h>

typedef struct
{
    uint32_t rtc_tick : 24;
    uint32_t timer_tick : 8;
} schedule_time_t;

typedef void (scheduled_callback_t)(void);

typedef struct scheduled_event_t
{
    volatile uint32_t * task;
    scheduled_callback_t * callback;
    schedule_time_t time;
} scheduled_event_t;


uint32_t scheduled_events_init(void);

uint32_t scheduled_events_schedule(scheduled_event_t * event);
uint32_t scheduled_events_cancel(scheduled_event_t * event); //TODO

uint32_t scheduled_events_get_current_time(schedule_time_t * current_time);


#endif
