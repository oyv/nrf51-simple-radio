#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdint.h>

#include "scheduled_events.h"

typedef struct linked_list_node_t 
{ 
    scheduled_event_t scheduled_event;
    struct linked_list_node_t * next; 
    uint16_t id;
} linked_list_node_t;

    
typedef struct linked_list_t
{
    linked_list_node_t root;
} linked_list_t;

uint32_t linked_list_init(linked_list_t * list);
uint32_t linked_list_insert_after(linked_list_node_t * new_node, linked_list_node_t * node_before);
uint32_t linked_list_insert(linked_list_t * list, linked_list_node_t * node, uint32_t index);
uint32_t linked_list_remove_index(linked_list_t * list, uint32_t index);
uint32_t linked_list_remove_node(linked_list_t * list, linked_list_node_t * node);
uint32_t linked_list_index_of(linked_list_t * list, linked_list_node_t * node, int32_t * out_index);
uint32_t linked_list_find_node(linked_list_t * list, int32_t index, linked_list_node_t ** out_node);
uint32_t linked_list_find_node_by_id(linked_list_t * list, uint16_t id, linked_list_node_t ** out_node);

#endif
