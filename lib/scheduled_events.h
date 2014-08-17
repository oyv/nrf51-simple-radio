#ifndef SCHEDULED_EVENT_H_
#define SCHEDULED_EVENT_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t rtc_tick;
    uint8_t timer_tick;
} schedule_time_t;

typedef void (scheduled_callback_t)(void);

typedef struct scheduled_event_t
{
    volatile uint32_t * task;
    scheduled_callback_t * callback;
    schedule_time_t time;
    uint16_t id;
    bool event_fired;
} scheduled_event_t;

typedef enum
{
    LESS_THAN,
    GREATER_THAN,
    EQUAL,
    UNCERTAIN,
} schedule_time_comparison_t;

uint32_t scheduled_events_init(void);

uint32_t scheduled_events_schedule(scheduled_event_t * event);
uint32_t scheduled_events_cancel(uint16_t id);

uint32_t scheduled_events_get_current_time(schedule_time_t * current_time);
schedule_time_comparison_t schedule_time_compare_rtc(schedule_time_t * time1, schedule_time_t * time2, schedule_time_t * current_time);
bool schedule_time_is_valid_rtc(schedule_time_t * time, schedule_time_t * current_time);



#endif
