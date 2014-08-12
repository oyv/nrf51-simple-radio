
#include "static_pool.h"
#include "scheduled_event.h"
#include "error.h"
#include "linked_list.h"
#include "nrf51.h"
#include "nrf51_bitfields.h"

#define N_SCHEDULED_EVENTS 10

static static_pool_t pool;
static scheduled_event_t pool_array[N_SCHEDULED_EVENTS];

static linked_list_t event_list;


uint32_t insert_sorted(linked_list_t * list, linked_list_node_t * node);

uint32_t scheduled_events_init()
{
    static_pool_init(&pool, (static_pool_node_t *)pool_array, N_SCHEDULED_EVENTS);
    linked_list_init(&event_list);
    
    NRF_RTC0->EVTEN = RTC_EVTEN_COMPARE0_Enabled << RTC_EVTEN_COMPARE0_Pos;
    
    NRF_PPI->CH[5].EEP = (uint32_t)&(NRF_RTC0->EVENTS_COMPARE[0]);
    NRF_PPI->CHEN = (PPI_CHEN_CH30_Enabled << PPI_CHEN_CH30_Pos) | (PPI_CHEN_CH31_Enabled << PPI_CHEN_CH31_Pos);
    
    NRF_RTC0->EVENTS_COMPARE[0] = 0;
    NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
    
    NVIC_SetPriority(RTC0_IRQn, 0);
    NVIC_EnableIRQ(RTC0_IRQn);
    
    NRF_RTC0->TASKS_CLEAR = 1;
    NRF_RTC0->TASKS_START = 1;
    
    return SUCCESS;
}

uint32_t scheduled_events_schedule(uint32_t * task, schedule_time_t time)
{
    uint32_t err_code;
    static_pool_node_t * new_event;
    err_code = static_pool_alloc(&pool, &new_event);
    if (err_code == SUCCESS)
    {
        new_event->list_node.scheduled_event.task = task;
        new_event->list_node.scheduled_event.time = time;
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
    NRF_PPI->CH[5].TEP = (uint32_t)scheduled_event->task;
    return SUCCESS;
}

uint32_t set_time(scheduled_event_t * scheduled_event)
{
    NRF_RTC0->CC[0] = scheduled_event->time;
    return SUCCESS;
}

uint32_t schedule_new_event()
{
    linked_list_node_t * new_event;
    uint32_t err_code = linked_list_find_node(&event_list, 0, &new_event);
    if (err_code == SUCCESS)
    {
        set_task(&(new_event->scheduled_event));
        set_time(&(new_event->scheduled_event));
        linked_list_remove_node(&event_list, new_event);
    }
    return err_code;
}

void RTC0_IRQHandler()
{
    if((NRF_RTC0->EVENTS_COMPARE[0] == 1) && (NRF_RTC0->INTENSET & RTC_INTENSET_COMPARE0_Msk))
    {
        NRF_RTC0->EVENTS_COMPARE[0] = 0;
        
        schedule_new_event();
    }
}
