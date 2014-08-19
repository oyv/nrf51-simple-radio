// #extern "C" 
// {
    #include "linked_list.h"
    #include "error.h"
// }

#include "CppUTest/TestHarness.h"

// #include <limits.h>
// #include <string.h>

// extern "C"
// {
    void error_handler(uint32_t err_code, uint32_t line_num, char * file_name)
    {

    }
// }

TEST_GROUP(linked_list)
{
    linked_list_t list;
    linked_list_node_t nodes[10];
    linked_list_node_t loose_node;

    void setup(void)
    {
       linked_list_init(&list);
        for (int i = 0; i < 10; i++)
        {
            nodes[i].id = 0;
        }
    }
    
    void teardown(void)
    {
    }
};

TEST(linked_list, initialization)
{
    linked_list_t list;
    uint32_t err_code = linked_list_init(&list);
    LONGS_EQUAL(SUCCESS, err_code);
    LONGS_EQUAL(0, list.root.next);    
}

TEST(linked_list, insert_after)
{
    uint32_t err_code;
    
    err_code = linked_list_insert_after(0, &list.root);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    err_code = linked_list_insert_after(&nodes[0], 0);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    
    err_code = linked_list_insert_after(&nodes[0], &list.root);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[0], &list.root.next);
    
    err_code = linked_list_insert_after(&nodes[1], &nodes[0]);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[1], nodes[0].next);
    
    err_code = linked_list_insert_after(&nodes[2], &nodes[1]);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[2], nodes[1].next);
    
    POINTERS_EQUAL(&nodes[2], list.root.next->next->next);
    
    CHECK(nodes[0].id != nodes[1].id);

}

TEST(linked_list, find)
{
    linked_list_insert_after(&nodes[0], &list.root);
    for (int i = 1; i < 10; i++)
        linked_list_insert_after(&nodes[i], &nodes[i-1]);

    linked_list_node_t * find_node;
    int32_t find_index;
    uint32_t err_code;
    
    // null pointer
    err_code = linked_list_find_node(0, 0, &find_node);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    err_code = linked_list_find_node(&list, 0, 0);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    
    // index out of range
    err_code = linked_list_find_node(&list, 11, &find_node);
    LONGS_EQUAL(ERROR_NOT_FOUND, err_code);
    
    // root node
    err_code = linked_list_find_node(&list, -1, &find_node);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&list.root, find_node);
    
    // first node
    err_code = linked_list_find_node(&list, 0, &find_node);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[0], find_node);
    
    // some node   
    err_code = linked_list_find_node(&list, 5, &find_node);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[5], find_node);
    
    // last node
    err_code = linked_list_find_node(&list, 9, &find_node);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[9], find_node);
    
    
    // null pointer
    err_code = linked_list_index_of(0, &nodes[0], &find_index);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    err_code = linked_list_index_of(&list, &nodes[0], 0);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    
    // node not in list
    err_code = linked_list_index_of(&list, &loose_node, &find_index);
    LONGS_EQUAL(ERROR_NOT_FOUND, err_code);
    
    // invalid pointer
    err_code = linked_list_index_of(&list, 0, &find_index);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    
    // root node
    err_code = linked_list_index_of(&list, &list.root, &find_index);
    LONGS_EQUAL(SUCCESS, err_code);
    LONGS_EQUAL(-1, find_index);
    
    // first node
    err_code = linked_list_index_of(&list, &nodes[0], &find_index);
    LONGS_EQUAL(SUCCESS, err_code);
    LONGS_EQUAL(0, find_index);
    
    // some node   
    err_code = linked_list_index_of(&list, &nodes[5], &find_index);
    LONGS_EQUAL(SUCCESS, err_code);
    LONGS_EQUAL(5, find_index);
    
    // last node
    err_code = linked_list_index_of(&list, &nodes[9], &find_index);
    LONGS_EQUAL(SUCCESS, err_code);
    LONGS_EQUAL(9, find_index);
    
    
    
    // null pointer
    err_code = linked_list_find_node_by_id(0, 0, &find_node);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    err_code = linked_list_find_node_by_id(&list, 0, 0);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    
    //find unused id
    uint16_t unused_id = 100;
    bool used = true;
    while (used = true)
    {
        used = false;
        for (int i = 0; i < 10; i++)
        {
            if (nodes[i].id == unused_id)
            {
                used = true;
            }
        }
    }
    
    // unused id
    err_code = linked_list_find_node_by_id(&list, unused_id, &find_node);
    LONGS_EQUAL(ERROR_NOT_FOUND, err_code);
    
    // first node
    err_code = linked_list_find_node_by_id(&list, nodes[0].id, &find_node);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[0], find_node);
    
    // some node   
    err_code = linked_list_find_node_by_id(&list, nodes[5].id, &find_node);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[5], find_node);
    
    // last node
    err_code = linked_list_find_node_by_id(&list, nodes[9].id, &find_node);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[9], find_node);

}

TEST(linked_list, insert)    
{
    uint32_t err_code;
    
    // null pointer
    err_code = linked_list_insert(0, &nodes[0], 1);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    err_code = linked_list_find_node_by_id(&list, 0, 0);
    LONGS_EQUAL(ERROR_INVALID, err_code);

    // index out of range
    err_code = linked_list_insert(&list, &nodes[0], 1);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    
    err_code = linked_list_insert(&list, &nodes[0], 0);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[0], &list.root.next);
    
    err_code = linked_list_insert(&list, &nodes[1], 0);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[0], nodes[1].next);
    
    err_code = linked_list_insert(&list, &nodes[2], 2);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[2], nodes[0].next);
}

TEST(linked_list, remove_index)
{
    linked_list_insert_after(&nodes[0], &list.root);
    for (int i = 1; i < 10; i++)
        linked_list_insert_after(&nodes[i], &nodes[i-1]);
        
     
    uint32_t err_code;
    
    // null pointer
    err_code = linked_list_remove_index(0, 0);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    
    // index out of range
    err_code = linked_list_remove_index(&list, 11);
    LONGS_EQUAL(ERROR_NOT_FOUND, err_code);
    err_code = linked_list_remove_index(&list, -1);
    LONGS_EQUAL(ERROR_NOT_FOUND, err_code);
    
    
    
    err_code = linked_list_remove_index(&list, 4);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[5], nodes[3].next);
    
    err_code = linked_list_remove_index(&list, 8);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(0, nodes[8].next);
    
    err_code = linked_list_remove_index(&list, 0);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[1], &list.root.next);
    
    for (int i = 0; i < 7; i++)
        err_code = linked_list_remove_index(&list, 0);
        
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(0, &list.root.next);
    
    err_code = linked_list_remove_index(&list, 0);
    LONGS_EQUAL(ERROR_NOT_FOUND, err_code);
}


TEST(linked_list, remove_node)
{
    linked_list_insert_after(&nodes[0], &list.root);
    for (int i = 1; i < 10; i++)
        linked_list_insert_after(&nodes[i], &nodes[i-1]);
        
     
    uint32_t err_code;
    
    // null pointer
    err_code = linked_list_remove_node(0, &nodes[0]);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    err_code = linked_list_remove_node(&list, 0);
    LONGS_EQUAL(ERROR_INVALID, err_code);
    
    // node not in list
    err_code = linked_list_remove_node(&list, &loose_node);
    LONGS_EQUAL(ERROR_NOT_FOUND, err_code);
    err_code = linked_list_remove_node(&list, &list.root);
    LONGS_EQUAL(ERROR_NOT_FOUND, err_code);
    
    
    
    err_code = linked_list_remove_node(&list, &nodes[4]);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[5], nodes[3].next);
    
    err_code = linked_list_remove_node(&list, &nodes[9]);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(0, nodes[8].next);
    
    err_code = linked_list_remove_node(&list, &nodes[0]);
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(&nodes[1], &list.root.next);
    
    err_code = linked_list_remove_node(&list, &nodes[1]);   
    err_code = linked_list_remove_node(&list, &nodes[2]);   
    err_code = linked_list_remove_node(&list, &nodes[3]);   
    err_code = linked_list_remove_node(&list, &nodes[5]);   
    err_code = linked_list_remove_node(&list, &nodes[6]);   
    err_code = linked_list_remove_node(&list, &nodes[7]);   
    err_code = linked_list_remove_node(&list, &nodes[8]); 
    
    LONGS_EQUAL(SUCCESS, err_code);
    POINTERS_EQUAL(0, &list.root.next);
    
    err_code = linked_list_remove_node(&list, &nodes[0]);
    LONGS_EQUAL(ERROR_NOT_FOUND, err_code);
}