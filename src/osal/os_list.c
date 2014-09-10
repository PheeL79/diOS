/***************************************************************************//**
* @file    os_list.c
* @brief   OS List.
* @author  A. Filyanov
*******************************************************************************/
#include "os_common.h"
#include "os_memory.h"
#include "os_list.h"

/******************************************************************************/
void OS_ListInit(OS_List* list_p)
{
    vListInitialise(list_p);
}

/******************************************************************************/
void OS_ListInsert(OS_List* list_p, OS_ListItem* new_item_l_p)
{
    vListInsert(list_p, new_item_l_p);
}

/******************************************************************************/
void OS_ListAppend(OS_List* list_p, OS_ListItem* new_item_l_p)
{
    vListInsertEnd(list_p, new_item_l_p);
}

/******************************************************************************/
U32 OS_ListRemove(OS_ListItem* item_l_p)
{
    return uxListRemove(item_l_p);
}

/******************************************************************************/
void OS_ListClear(OS_List* list_p)
{
    if (OS_NULL == list_p) { return; }
    if (OS_ListIsEmpty(list_p)) { return; }
    OS_ListItem* iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(list_p));
    while (OS_DELAY_MAX != (OS_Value)OS_ListItemValueGet(iter_li_p)) {
        OS_ListItem* iter_li_tmp_p = iter_li_p;
        iter_li_p = OS_ListItemNextGet(iter_li_p);
        OS_ListItemDelete(iter_li_tmp_p);
    }
}

/******************************************************************************/
OS_ListItem* OS_ListItemCreate(void)
{
OS_ListItem* item_l_p = OS_Malloc(sizeof(OS_ListItem));
    if (OS_NULL != item_l_p) {
        OS_ListItemInit(item_l_p);
        return item_l_p;
    }
    return OS_NULL;
}

/******************************************************************************/
void OS_ListItemDelete(OS_ListItem* item_l_p)
{
    OS_ListRemove(item_l_p);
    OS_Free(item_l_p);
}

/******************************************************************************/
void OS_ListItemInit(OS_ListItem* item_l_p)
{
    vListInitialiseItem(item_l_p);
}

/******************************************************************************/
OS_ListItem* OS_ListItemByValueGet(OS_List* list_p, const OS_Value value)
{
OS_ListItem* iter_li_p;

    if (OS_NULL == list_p) { return OS_NULL; }
    if (OS_ListIsEmpty(list_p)) { return OS_NULL; }
    for (iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(list_p));
         value != (OS_Value)OS_ListItemValueGet(iter_li_p);
         iter_li_p = OS_ListItemNextGet(iter_li_p)) {
        if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) { return OS_NULL; }
    }
    return (OS_ListItem* )iter_li_p;
}

/******************************************************************************/
OS_ListItem* OS_ListItemByOwnerGet(OS_List* list_p, const OS_Owner owner)
{
OS_ListItem* iter_li_p;

    if (OS_NULL == list_p) { return OS_NULL; }
    if (OS_ListIsEmpty(list_p)) { return OS_NULL; }
    for (iter_li_p = OS_ListItemNextGet((OS_ListItem*)&OS_ListItemLastGet(list_p));
         owner != (OS_Owner*)OS_ListItemOwnerGet(iter_li_p);
         iter_li_p = OS_ListItemNextGet(iter_li_p)) {
        if (OS_DELAY_MAX == OS_ListItemValueGet(iter_li_p)) { return OS_NULL; }
    }
    return (OS_ListItem*)iter_li_p;
}

/******************************************************************************/
/*
void swapNodes(Node* left, Node* right)
{
    if(left->prev)  left->prev->next    = right;
    if(right->prev) right->prev->next   = left;
    if(left->next)  left->next->prev    = right;
    if(right->next) right->next->prev   = left;
    Node* temp;
    temp = left->prev;
    left->prev = right->prev;
    right->prev = temp;
    temp = left->next;
    left->next= right->next;
    right->next= temp;
}
*/
void OS_ListItemsSwap(OS_ListItem* item_1_p, OS_ListItem* item_2_p)
{
    if (OS_ListItemPreviousGet(item_2_p) != item_1_p) {
        OS_ListItemNextGet(OS_ListItemPreviousGet(item_2_p)) = item_1_p;
        OS_ListItemPreviousGet(OS_ListItemNextGet(item_1_p)) = item_2_p;
    } else {
        OS_ListItemNextGet(OS_ListItemPreviousGet(item_2_p)) = OS_ListItemNextGet(item_2_p);
        OS_ListItemPreviousGet(OS_ListItemNextGet(item_1_p)) = item_1_p;
    }
    OS_ListItemNextGet(OS_ListItemPreviousGet(item_1_p)) = item_2_p;
    OS_ListItemPreviousGet(OS_ListItemNextGet(item_2_p)) = item_1_p;
    OS_ListItem* item_tmp_p = OS_ListItemPreviousGet(item_1_p);
    if (OS_ListItemPreviousGet(item_2_p) != item_1_p) {
        OS_ListItemPreviousGet(item_1_p) = OS_ListItemPreviousGet(item_2_p);
    } else {
        OS_ListItemPreviousGet(item_1_p) = item_2_p;
    }
    OS_ListItemPreviousGet(item_2_p) = item_tmp_p;
    item_tmp_p = OS_ListItemNextGet(item_1_p);
    OS_ListItemNextGet(item_1_p) = OS_ListItemNextGet(item_2_p);
    if (OS_ListItemNextGet(item_2_p) != item_tmp_p) {
        OS_ListItemNextGet(item_2_p) = item_tmp_p;
    } else {
        OS_ListItemNextGet(item_2_p) = item_1_p;
    }
/*
OS_ListItem* item_1p_p = OS_ListItemPreviousGet(item_1_p);
OS_ListItem* item_1n_p = OS_ListItemNextGet(item_1_p);
OS_ListItem* item_2p_p = OS_ListItemPreviousGet(item_2_p);
OS_ListItem* item_2n_p = OS_ListItemNextGet(item_2_p);
    if (item_2_p != item_1n_p) {
        OS_ListItemNextGet(item_2_p) = item_1n_p;
    } else {
        OS_ListItemNextGet(item_2_p) = item_1_p;
    }
    if (item_1_p != item_2p_p) {
        OS_ListItemPreviousGet(item_1_p) = item_2p_p;
    } else {
        OS_ListItemPreviousGet(item_1_p) = item_2_p;
    }
    if (item_1n_p != item_2_p) {
        OS_ListItemPreviousGet(item_1n_p)= item_2_p;
    }
    if (item_2p_p != item_1_p) {
        OS_ListItemNextGet(item_2p_p)= item_1_p;
    }
    OS_ListItemNextGet(item_1_p)     = item_2n_p;
    OS_ListItemPreviousGet(item_2_p) = item_1p_p;
    OS_ListItemNextGet(item_1p_p)    = item_2_p;
    OS_ListItemPreviousGet(item_2n_p)= item_1_p;
*/
}