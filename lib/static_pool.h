#ifndef STATIC_POOL_H_
#define STATIC_POOL_H_

#include <stdbool.h>
#include <stdint.h>

#include "linked_list.h"

typedef union static_pool_node_t
{
    linked_list_node_t list_node;
} static_pool_node_t;

#define POOL_MAX_SIZE 10


typedef struct static_pool_t        
{
    uint32_t n_nodes;
    static_pool_node_t * nodes;
    bool allocated[POOL_MAX_SIZE];
} static_pool_t;


uint32_t static_pool_init(static_pool_t * pool, static_pool_node_t * array, uint32_t array_size);
uint32_t static_pool_alloc(static_pool_t * pool, static_pool_node_t ** node);
uint32_t static_pool_free(static_pool_t * pool, static_pool_node_t * node);

#endif
