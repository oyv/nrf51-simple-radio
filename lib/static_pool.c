#include "static_pool.h"
#include "error.h"

uint32_t static_pool_init(static_pool_t * pool, static_pool_node_t * array, uint32_t n_nodes)
{
    pool->n_nodes = n_nodes;
    pool->nodes = array;
    for (int i = 0; i < pool->n_nodes; i++)
        pool->allocated[i] = false;
    return SUCCESS;
}

uint32_t static_pool_alloc(static_pool_t * pool, static_pool_node_t ** node)
{
    int i;
    for (i = 0; (i < pool->n_nodes); i++)
    {
        if (!(pool->allocated[i]))
        {
            *node = &pool->nodes[i];
            pool->allocated[i] = true;
            return SUCCESS;
        }
    }
    return ERROR_NO_MEMORY;
}

uint16_t index_of(static_pool_t * pool, static_pool_node_t * node)
{
    return(node - &(pool->nodes[0])) / sizeof(pool->nodes[0]);
}

uint32_t static_pool_free(static_pool_t * pool, static_pool_node_t * node)
{
    uint16_t index = index_of(pool, node);
    if (pool->allocated[index])
    {
        pool->allocated[index] = false;
        return SUCCESS;
    }
    else
    {
        return ERROR_NOT_FOUND;
    }
}
