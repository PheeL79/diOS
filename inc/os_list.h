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
#define OS_ListCurrentLenGet                            listCURRENT_LIST_LENGTH
#define OS_ListIsEmpty                                  listLIST_IS_EMPTY
#define OS_ListItemValueFirstGet                        listGET_ITEM_VALUE_OF_HEAD_ENTRY
#define OS_ListItemValueGet                             listGET_LIST_ITEM_VALUE
#define OS_ListItemValueSet                             listSET_LIST_ITEM_VALUE
#define OS_ListItemOwnerGet                             listGET_LIST_ITEM_OWNER
#define OS_ListItemOwnerSet                             listSET_LIST_ITEM_OWNER
#define OS_ListItemPreviousGet(os_list_item_p)          ((os_list_item_p)->pxPrevious)
#define OS_ListItemNextGet                              listGET_NEXT
#define OS_ListItemFirstGet                             listGET_HEAD_ENTRY
#define OS_ListItemLastGet(os_list_p)                   ((os_list_p)->xListEnd)
#define OS_ListItemOwnerNextGet                         listGET_OWNER_OF_NEXT_ENTRY
#define OS_ListItemOwnerFirstGet                        listGET_OWNER_OF_HEAD_ENTRY
#define OS_ListItemIsWithin                             listIS_CONTAINED_WITHIN
#define OS_ListItemContainerGet                         listLIST_ITEM_CONTAINER
#define OS_ListIsInitialised                            listLIST_IS_INITIALISED

//------------------------------------------------------------------------------
typedef List_t OS_List;
typedef ListItem_t OS_ListItem;
typedef MiniListItem_t OS_ListItemLight;

/// @brief      Initialise the list.
/// @param[in]  list_p          List.
/// @return     None.
void            OS_ListInit(OS_List* list_p);

/// @brief      Clear the list.
/// @param[in]  list_p          List.
/// @return     None.
/// @note       Removes all items from the list and destroy them.
void            OS_ListClear(OS_List* list_p);

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

/// @brief      Get list item by it's value.
/// @param[in]  list_p          List.
/// @param[in]  value           Value.
/// @return     List item.
OS_ListItem*    OS_ListItemByValueGet(OS_List* list_p, const OS_Value value);

/// @brief      Get list item by it's owner.
/// @param[in]  list_p          List.
/// @param[in]  owner           Owner.
/// @return     List item.
OS_ListItem*    OS_ListItemByOwnerGet(OS_List* list_p, const OS_Owner owner);

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
