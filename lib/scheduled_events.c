
#include <string.h>

#include "static_pool.h"
#include "scheduled_events.h"
#include "error.h"
#include "linked_list.h"
#include "nrf51.h"
#include "nrf51_bitfields.h"

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

static linked_list_t event_list;

static scheduled_event_t * current_event;
static scheduled_event_t * next_event;

static scheduled_callback_t * current_callback;


uint32_t insert_sorted(linked_list_t * list, linked_list_node_t * node);

uint32_t scheduled_events_init()
{
    static_pool_init(&pool, (static_pool_node_t *)pool_array, N_SCHEDULED_EVENTS);
    linked_list_init(&event_list);
    
    NRF_RTC0->EVTEN = RTC_EVTEN_COMPARE0_Enabled << RTC_EVTEN_COMPARE0_Pos;
    
    NRF_RTC0->EVENTS_COMPARE[SCHEDULE_CC_REG_RTC] = 0;
    NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
    
    NVIC_SetPriority(RTC0_IRQn, 2);
    NVIC_EnableIRQ(RTC0_IRQn);
    
    NRF_RTC0->TASKS_CLEAR = 1;
    NRF_RTC0->TASKS_START = 1;

    NRF_TIMER2->EVENTS_COMPARE[SCHEDULE_CC_REG_TIMER] = 0;
    NRF_TIMER2->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
    
    NVIC_SetPriority(TIMER2_IRQn, 2);
    NVIC_EnableIRQ(TIMER2_IRQn);
    
    NRF_TIMER2->TASKS_STOP = 1;
    
    
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
    
    NRF_PPI->CHENSET = (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);
    
    return SUCCESS;
}

uint32_t scheduled_events_schedule(scheduled_event_t * event)
{
    uint32_t err_code;
    static_pool_node_t * new_event;
    err_code = static_pool_alloc(&pool, &new_event);
    if (err_code == SUCCESS)
    {
        memcpy(new_event, event, sizeof(new_event));
        insert_sorted(&event_list, &new_event->list_node);
    }
    return err_code;
}


uint32_t insert_sorted(linked_list_t * list, linked_list_node_t * node)
{
    uint32_t err_code;
    linked_list_node_t * node_it;
    
    for (   node_it = &event_list.root; 
           (node_it->next != 0) && (node_it->next->scheduled_event.time < node->scheduled_event.time); 
            node_it = node_it->next) 
        {;}
        
    err_code = linked_list_insert_after(node, node_it);
    return err_code;
}

uint32_t set_task(scheduled_event_t * scheduled_event)
{
    // TODO: handle task == 0
    NRF_PPI->CH[5].TEP = (uint32_t)scheduled_event->task;
    return SUCCESS;
}

uint32_t set_time(scheduled_event_t * scheduled_event)
{
    
    // TODO: handle timer ticks
    NRF_RTC0->CC[SCHEDULE_CC_REG_RTC] = scheduled_event->time.rtc_tick;
    return SUCCESS;
}

uint32_t set_callback(scheduled_callback_t * callback)
{
    current_callback = callback;
    return SUCCESS;
}

uint32_t schedule_new_event()
{
    linked_list_node_t * new_event;
    uint32_t err_code = linked_list_find_node(&event_list, 0, &new_event);
    
    if (err_code == SUCCESS)
    {
        next_event = new_event;
        
        set_task(&(new_event->scheduled_event));
        set_time(&(new_event->scheduled_event));
        set_callback(&(new_event->scheduled_callback));
        linked_list_remove_node(&event_list, new_event);
    }
    return err_code;
}

uint32_t scheduled_events_get_current_time(scheduled_time_t * current_time)
{
    NRF_RTC0->TASKS_CAPTURE[GET_TIME_CC_REG_RTC] = 1;
    NRF_TIMER2->TASKS_CAPTURE[GET_TIME_CC_REG_TIMER] = 1;
    
    current_time->rtc_tick = NRF_RTC0->CC[GET_TIME_CC_REG_RTC];
    current_time->timer_tick = NRF_TIMER2->CC[GET_TIME_CC_REG_TIMER];
    
    return SUCCESS;
}

uint32_t get_captured_time(scheduled_time_t * captured_time)
{
    captured_time->rtc_tick = NRF_RTC0->CC[CAPTURE_TIME_CC_REG_RTC];
    captured_time->timer_tick = NRF_TIMER2->CC[CAPTURE_TIME_CC_REG_TIMER];
}

uint32_t scheduled_time_reached()
{    
    current_event = next_event;
    
    (*current_callback)();
    schedule_new_event();
}

void RTC0_IRQHandler()
{
    if((NRF_RTC0->EVENTS_COMPARE[SCHEDULE_CC_REG_RTC] == 1) && (NRF_RTC0->INTENSET & RTC_INTENSET_COMPARE0_Msk))
    {
        NRF_RTC0->EVENTS_COMPARE[SCHEDULE_CC_REG_RTC] = 0;
        
        if (NRF_TIMER2->CC[SCHEDULE_CC_REG_TIMER] == 0)
        {
            scheduled_time_reached();
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
            scheduled_time_reached();
        }
    }
}
