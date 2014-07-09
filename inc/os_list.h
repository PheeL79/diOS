/***************************************************************************//**
* @file    os_list.h
* @brief   OS List.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_LIST_H_
#define _OS_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "list.h"
#include "typedefs.h"
#include "os_common.h"

/**
* \defgroup OS_List OS_List
* @{
*/
//------------------------------------------------------------------------------
#define OS_LIST_CURRENT_LEN_GET(OS_ListP)               listCURRENT_LIST_LENGTH(OS_ListP)
#define OS_LIST_IS_EMPTY(OS_ListP)                      listLIST_IS_EMPTY(OS_ListP)
#define OS_LIST_ITEM_FIRST_VALUE_GET(OS_ListP)          listGET_ITEM_VALUE_OF_HEAD_ENTRY(OS_ListP)
#define OS_LIST_ITEM_VALUE_GET(OS_ListItemP)            listGET_LIST_ITEM_VALUE(OS_ListItemP)
#define OS_LIST_ITEM_VALUE_SET(OS_ListItemP, OS_Value)  listSET_LIST_ITEM_VALUE(OS_ListItemP, OS_Value)
#define OS_LIST_ITEM_OWNER_GET(OS_ListItemP)            listGET_LIST_ITEM_OWNER(OS_ListItemP)
#define OS_LIST_ITEM_OWNER_SET(OS_ListItemP, OS_Owner)  listSET_LIST_ITEM_OWNER(OS_ListItemP, OS_Owner)
#define OS_LIST_ITEM_PREVIOUS_GET(OS_ListItemP)         ((OS_ListItemP)->pxPrevious)
#define OS_LIST_ITEM_NEXT_GET(OS_ListItemP)             listGET_NEXT(OS_ListItemP)
#define OS_LIST_ITEM_FIRST_GET(OS_ListP)                listGET_HEAD_ENTRY(OS_ListP)
#define OS_LIST_ITEM_LAST_GET(OS_ListP)                 ((OS_ListP)->xListEnd)
#define OS_LIST_ITEM_NEXT_OWNER_GET(OS_Owner, OS_ListP) listGET_OWNER_OF_NEXT_ENTRY(OS_Owner, OS_ListP)
#define OS_LIST_ITEM_FIRST_OWNER_GET(OS_ListP)          listGET_OWNER_OF_HEAD_ENTRY(OS_ListP)
#define OS_LIST_ITEM_IS_WITHIN(OS_ListP, OS_ListItemP)  listIS_CONTAINED_WITHIN(OS_ListP, OS_ListItemP)
#define OS_LIST_ITEM_CONTAINER_GET(OS_ListItemP)        listLIST_ITEM_CONTAINER(OS_ListItemP)
#define OS_LIST_IS_INITIALISED(OS_ListP)                listLIST_IS_INITIALISED(OS_ListP)

//------------------------------------------------------------------------------
typedef List_t OS_List;
typedef ListItem_t OS_ListItem;
typedef MiniListItem_t OS_ListItemLight;

/// @brief      Initialise the list.
/// @param[in]  list_p          List.
/// @return     None.
void            OS_ListInit(OS_List* list_p);

/// @brief      Create and initialize the list item.
/// @return     List item.
OS_ListItem*    OS_ListItemCreate(void);

/// @brief      Remove and delete item from the list.
/// @param[in]  item_l_p        List item.
/// @return     None.
void            OS_ListItemDelete(OS_ListItem* item_l_p);

/// @brief      Initialise the list item.
/// @param[in]  item_l_p        List item.
/// @return     None.
void            OS_ListItemInit(OS_ListItem* item_l_p);

/// @brief      Insert item to the list.
/// @param[in]  list_p          List.
/// @param[in]  new_item_l_p    New list item.
/// @return     None.
void            OS_ListInsert(OS_List* list_p, OS_ListItem* new_item_l_p);

/// @brief      Append item to the list.
/// @param[in]  list_p          List.
/// @param[in]  new_item_l_p    New list item.
/// @return     None.
void            OS_ListAppend(OS_List* list_p, OS_ListItem* new_item_l_p);

/// @brief      Remove item from the list.
/// @param[in]  item_l_p        List item.
/// @return     The number of items that remain in the list.
U32             OS_ListRemove(OS_ListItem* item_l_p);

/// @brief      Find list item by it's value.
/// @param[in]  list_p          List.
/// @param[in]  value           Value.
/// @return     List item.
OS_ListItem*    OS_ListItemByValueFind(OS_List* list_p, const OS_Value value);

/// @brief      Find list item by it's owner.
/// @param[in]  list_p          List.
/// @param[in]  owner           Owner.
/// @return     List item.
OS_ListItem*    OS_ListItemByOwnerFind(OS_List* list_p, const OS_Owner owner);

/// @brief      Swap list items.
/// @param[in]  item_1_p        List item first.
/// @param[in]  item_2_p        List item second.
/// @return     None.
void            OS_ListItemsSwap(OS_ListItem* item_1_p, OS_ListItem* item_2_p);

/**@}*/ //OS_List

#ifdef __cplusplus
}
#endif

#endif // _OS_LIST_H_
