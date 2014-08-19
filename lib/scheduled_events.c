
#include <string.h>

#include "static_pool.h"
#include "scheduled_events.h"
#include "error.h"
#include "linked_list.h"
#include "nrf51.h"
#include "nrf51_bitfields.h"

#define POW_2_24 0x1000000
#define INVALID_FUTURE_POINT 0x800000

#define N_SCHEDULED_EVENTS 10

#define SCHEDULE_CC_REG_RTC 0
#define SCHEDULE_CC_REG_TIMER 0
#define GET_TIME_CC_REG_RTC 1
#define GET_TIME_CC_REG_TIMER 1
#define CAPTURE_TIME_CC_REG_RTC 2
#define CAPTURE_TIME_CC_REG_TIMER 2
#define RESET_CC_REG_TIMER 2

static static_pool_t pool;
static scheduled_event_t pool_array[N_SCHEDULED_EVENTS];

linked_list_t event_list;

static scheduled_event_t * next_event = 0;

static scheduled_callback_t * current_callback;



uint32_t schedule_new_event(void);
uint32_t insert_sorted(linked_list_t * list, linked_list_node_t * node, int32_t *index_out);
uint32_t event_rollover(void);

uint32_t scheduled_events_init()
{
    static_pool_init(&pool, (static_pool_node_t *)pool_array, N_SCHEDULED_EVENTS);
    linked_list_init(&event_list);
    
    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    
    NRF_RTC0->EVTEN = RTC_EVTEN_COMPARE0_Enabled << RTC_EVTEN_COMPARE0_Pos;
    
    NRF_RTC0->EVENTS_COMPARE[SCHEDULE_CC_REG_RTC] = 0;
    NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
    
    NVIC_SetPriority(RTC0_IRQn, 2);
    NVIC_EnableIRQ(RTC0_IRQn);
    
    NRF_RTC0->TASKS_CLEAR = 1;
    NRF_RTC0->TASKS_START = 1;

    NRF_TIMER2->EVENTS_COMPARE[SCHEDULE_CC_REG_TIMER] = 0;
    NRF_TIMER2->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
    
    NRF_TIMER2->TASKS_STOP = 1;
    NRF_TIMER2->TASKS_CLEAR = 1;
    
    NVIC_SetPriority(TIMER2_IRQn, 2);
    NVIC_EnableIRQ(TIMER2_IRQn);
    
    
    
    // Schedulable task goes here
    NRF_PPI->CH[5].EEP = (uint32_t)&(NRF_RTC0->EVENTS_COMPARE[SCHEDULE_CC_REG_RTC]);
    
    // Start HF timer for granularity
    NRF_PPI->CH[6].EEP = (uint32_t)&(NRF_RTC0->EVENTS_COMPARE[SCHEDULE_CC_REG_RTC]);
    NRF_PPI->CH[6].TEP = (uint32_t)&(NRF_TIMER2->TASKS_START);
    
    // Stop timer after one RTC tick
    NRF_TIMER2->CC[RESET_CC_REG_TIMER] = 35;
    NRF_PPI->CH[7].EEP = (uint32_t)&(NRF_TIMER2->EVENTS_COMPARE[RESET_CC_REG_TIMER]);
    NRF_PPI->CH[7].TEP = (uint32_t)&(NRF_TIMER2->TASKS_STOP);
    NRF_PPI->CH[8].EEP = (uint32_t)&(NRF_TIMER2->EVENTS_COMPARE[RESET_CC_REG_TIMER]);
    NRF_PPI->CH[8].TEP = (uint32_t)&(NRF_TIMER2->TASKS_CLEAR);
    
    NRF_PPI->CHENSET = (1 << 7) | (1 << 8);
    
    return SUCCESS;
}

uint32_t scheduled_events_schedule(scheduled_event_t * event)
{
    uint32_t err_code;
    int32_t new_index;
    linked_list_node_t * new_event;
    err_code = static_pool_alloc(&pool, &new_event);
    if (err_code == SUCCESS)
    {
        new_event->scheduled_event = *event;
        insert_sorted(&event_list, new_event, &new_index);
        event->id = new_event->id;
        new_event->scheduled_event.id = new_event->id;
        
        if (new_index == 0)
        {
            schedule_new_event();
        }
    }
    return err_code;
}

uint32_t remove_event(linked_list_node_t * event_to_remove)
{
    int32_t index, err_code;
    
    err_code = linked_list_index_of(&event_list, event_to_remove, &index);
    if (err_code == SUCCESS && index == 0)
    {
        NRF_PPI->CHENCLR = (1 << 5);
        if (!event_to_remove->scheduled_event.event_fired)
        {
            err_code = event_rollover();
        }
    }
    else if (err_code == SUCCESS)
    {
        err_code = linked_list_remove_node(&event_list, event_to_remove);
        if (err_code == SUCCESS)
            err_code = static_pool_free(&pool, (static_pool_node_t*)event_to_remove);
    }
    
    return err_code;
}

uint32_t scheduled_events_cancel(uint16_t id)
{
    linked_list_node_t * event_to_cancel;
    uint32_t err_code = linked_list_find_node_by_id(&event_list, id, &event_to_cancel);
    if (err_code == SUCCESS)
    {
        err_code = remove_event(event_to_cancel);
    }
    return err_code;
}

uint32_t normalize_time(schedule_time_t * time, schedule_time_t * current_time)
{
    if (current_time == 0)
        scheduled_events_get_current_time(current_time);
    
    uint32_t time_rtc_tick = time->rtc_tick;
    uint32_t current_rtc_tick = current_time->rtc_tick;
    
    return ((time_rtc_tick + POW_2_24 - current_rtc_tick) % POW_2_24);
}

bool schedule_time_is_valid_normalized(uint32_t time_normalized)
{
    return time_normalized < INVALID_FUTURE_POINT;
}

bool schedule_time_is_valid_rtc(schedule_time_t * time, schedule_time_t * current_time)
{
    return schedule_time_is_valid_normalized(normalize_time(time, current_time));
}

schedule_time_comparison_t schedule_time_compare_normalized(uint32_t time1_normalized, uint32_t time2_normalized)
{
    if (!schedule_time_is_valid_normalized(time1_normalized) || !schedule_time_is_valid_normalized(time2_normalized))
        return UNCERTAIN;
    else if (time1_normalized == time2_normalized)
        return EQUAL;
    else if (time1_normalized > time2_normalized)
        return GREATER_THAN;
    else
        return LESS_THAN;
}

schedule_time_comparison_t schedule_time_compare_rtc(schedule_time_t * time1, schedule_time_t * time2, schedule_time_t * current_time)
{
    if (current_time == 0)
        scheduled_events_get_current_time(current_time);
    
    uint32_t time1_normalized = normalize_time(time1, current_time);
    uint32_t time2_normalized = normalize_time(time2, current_time);
    
    return schedule_time_compare_normalized(time1_normalized, time2_normalized);
}


uint32_t insert_sorted(linked_list_t * list, linked_list_node_t * node, int32_t *index_out)
{
    uint32_t err_code;
    linked_list_node_t * node_it;
    
    schedule_time_t current_time;
    scheduled_events_get_current_time(&current_time);
    
    *index_out = -1;
        
    bool done = false;
    for (node_it = &list->root; (node_it != 0) && !done; node_it = node_it->next)
    {
        (*index_out)++;
        
        if (node_it->next == 0)
        {
            err_code = linked_list_insert_after(node, node_it);
            done = true;
        }
        else
        {
            schedule_time_comparison_t comparison = schedule_time_compare_rtc(&(node->scheduled_event.time), &(node_it->next->scheduled_event.time), &current_time);
            switch(comparison)
            {
                case GREATER_THAN:
                    // keep going
                    break;
                case LESS_THAN:
                    // sorting done
                    err_code = linked_list_insert_after(node, node_it);
                    done = true;
                    break;
                case EQUAL:
                    // no events on the same RTC tick allowed
                case UNCERTAIN:
                    // no invalid times allowed
                default:
                    err_code = ERROR_INVALID;
                    done = true;
                    break;
            }
        }
    }
    return err_code;
}

uint32_t set_task(scheduled_event_t * scheduled_event)
{
    if (scheduled_event->task == 0)
        NRF_PPI->CHENCLR = (1 << 5);
    else
    {
        NRF_PPI->CH[5].TEP = (uint32_t)scheduled_event->task;
        NRF_PPI->CHENSET = (1 << 5);
    }
    return SUCCESS;
}

uint32_t set_time(scheduled_event_t * scheduled_event)
{
    NRF_RTC0->CC[SCHEDULE_CC_REG_RTC] = scheduled_event->time.rtc_tick;
    NRF_TIMER2->CC[SCHEDULE_CC_REG_TIMER] = scheduled_event->time.timer_tick;
    
    if (scheduled_event->time.timer_tick == 0)
        NRF_PPI->CHENCLR = (1 << 6);
    else
        NRF_PPI->CHENSET = (1 << 6);
    return SUCCESS;
}

uint32_t set_callback(scheduled_event_t * scheduled_event)
{
    current_callback = scheduled_event->callback;
    return SUCCESS;
}

uint32_t schedule_new_event()
{
    linked_list_node_t * node;
    uint32_t err_code;
    err_code = linked_list_find_node(&event_list, 0, &node);
    
    if (err_code == SUCCESS)
    {
        next_event = &node->scheduled_event;
        
        set_task(next_event);
        set_time(next_event);
        set_callback(next_event);
    }
    return err_code;
}

uint32_t scheduled_events_get_current_time(schedule_time_t * current_time)
{
    current_time->rtc_tick = NRF_RTC0->COUNTER;
    
    NRF_TIMER2->TASKS_CAPTURE[GET_TIME_CC_REG_TIMER] = 1;
    current_time->timer_tick = NRF_TIMER2->CC[GET_TIME_CC_REG_TIMER];
    
    return SUCCESS;
}

uint32_t get_captured_time(schedule_time_t * captured_time)
{
    captured_time->rtc_tick = NRF_RTC0->CC[CAPTURE_TIME_CC_REG_RTC];
    captured_time->timer_tick = NRF_TIMER2->CC[CAPTURE_TIME_CC_REG_TIMER];
    
    return SUCCESS;
}

uint32_t event_rollover()
{
    uint32_t err_code;
    
    static scheduled_event_t * current_event = 0;
    scheduled_event_t * old_current_event = current_event;
    current_event = next_event;
    
    if (current_event && current_event->event_fired && current_callback != 0)
        (*current_callback)();
    
    err_code = schedule_new_event();

    if (current_event != 0)
        err_code = linked_list_remove_node(&event_list, (linked_list_node_t *)current_event);
        
    if (old_current_event != 0)
        err_code = static_pool_free(&pool, (static_pool_node_t*)old_current_event);
    
    return err_code;
}

void RTC0_IRQHandler()
{
    if((NRF_RTC0->EVENTS_COMPARE[SCHEDULE_CC_REG_RTC] == 1) && (NRF_RTC0->INTENSET & RTC_INTENSET_COMPARE0_Msk))
    {
        NRF_RTC0->EVENTS_COMPARE[SCHEDULE_CC_REG_RTC] = 0;
        
        if (NRF_TIMER2->CC[SCHEDULE_CC_REG_TIMER] == 0)
        {
            next_event->event_fired = true;
            event_rollover();
        }
    }
}

void TIMER2_IRQHandler()
{
    if((NRF_TIMER2->EVENTS_COMPARE[SCHEDULE_CC_REG_TIMER] == 1) && (NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk))
    {
        NRF_TIMER2->EVENTS_COMPARE[SCHEDULE_CC_REG_TIMER] = 0;
        
        if (NRF_TIMER2->CC[SCHEDULE_CC_REG_TIMER] != 0)
        {
            next_event->event_fired = true;
            event_rollover();
        }
    }
}
