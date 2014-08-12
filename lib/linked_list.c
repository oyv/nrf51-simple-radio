
#include "linked_list.h"
#include "error.h"


uint32_t linked_list_init(linked_list_t * list)
{
    list->root.next = 0;
    return SUCCESS;
}

uint32_t linked_list_insert_after(linked_list_node_t * new_node, linked_list_node_t * node_before)
{
    new_node->next = node_before->next;
    node_before->next = new_node;
    return SUCCESS;
}


uint32_t linked_list_insert(linked_list_t * list, linked_list_node_t * node, uint32_t index)
{
    uint32_t err_code = SUCCESS;
    linked_list_node_t * node_before;
 
    err_code = linked_list_find_node(list, (int32_t)(index)-1, &node_before);
    
    if (err_code == SUCCESS)
    {
        err_code = linked_list_insert_after(node, node_before);
    }
    return err_code;
}

uint32_t remove_next_node(linked_list_node_t * node)
{
    if (node->next == 0)
    {
        return ERROR_NOT_FOUND;
    }
    node->next = node->next->next;
    return SUCCESS;
}

uint32_t linked_list_remove_index(linked_list_t * list, uint32_t index)
{
    uint32_t err_code;
    linked_list_node_t * node_before;
    err_code = linked_list_find_node(list, (int32_t)(index) - 1, &node_before);
    if (err_code == SUCCESS)
    {
        err_code = remove_next_node(node_before);
    }
    return err_code;
}


uint32_t linked_list_index_of(linked_list_t * list, linked_list_node_t * node, int32_t * out_index)
{
    linked_list_node_t * node_it;
    for (node_it = &list->root; (node_it == node) || (node_it == 0); node_it = node_it->next)
    {
        (*out_index)++;
    }
    if (node_it == 0)
        return ERROR_NOT_FOUND;
    else
        return SUCCESS;
}

uint32_t linked_list_find_node(linked_list_t * list, int32_t index, linked_list_node_t ** out_node)
{
    // index can be -1 -> list->root
    
    *out_node = &list->root;
    for (int32_t i = -1; i < index; i++)
    {
        *out_node = (*out_node)->next;
        if (*out_node == 0)
            return ERROR_NOT_FOUND;
    }
    return SUCCESS;
}

