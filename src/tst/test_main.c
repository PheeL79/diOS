//-----------------------------------------------------------------------------
#include "os_config.h"

#ifdef TEST
#include <stdlib.h>
#include "unity.h"
#include "common.h"
#include "hal.h"
#include "test_main.h"
#include "os_debug.h"
#include "os_list.h"
#include "os_memory.h"

//-----------------------------------------------------------------------------
extern void setUp(void);
extern void tearDown(void);

static void TestList(void);
static void TestListSort(const OS_List* list_p, const SortDirection sort_dir);
static void TestListLog(const OS_List* list_p);

//-----------------------------------------------------------------------------
static void runTest(UnityTestFunction test);
void resetTest(void);

/******************************************************************************/
void TestsRun(void)
{
    UnityBegin();
    RUN_TEST(TestList, 1);
    UnityEnd();
}

/******************************************************************************/
void setUp(void)
{
}

/******************************************************************************/
void tearDown(void)
{
}

///******************************************************************************/
static void runTest(UnityTestFunction test)
{
    if (TEST_PROTECT()) {
        setUp();
        test();
    }
    if (TEST_PROTECT() && !TEST_IS_IGNORED) {
        tearDown();
    }
}

///******************************************************************************/
void resetTest(void)
{
    tearDown();
    setUp();
}

/******************************************************************************/
void TestList(void)
{
OS_List os_test_list;
OS_ListItem* item_l_p;
OS_ListItem* item_next_p;
    // Init list.
    srand(rand());
    OS_ListInit(&os_test_list);
    TEST_ASSERT_TRUE(OS_LIST_IS_INITIALISED(&os_test_list));
    // Add items to the list.
    for (SIZE i = 0; i < 0x10; ++i) {
        item_l_p = OS_ListItemCreate();
        if (OS_NULL == item_l_p) {
            OS_TRACE(D_CRITICAL, "No memory!\n", OS_NULL);
        }
        OS_LIST_ITEM_VALUE_SET(item_l_p, (OS_Value)(rand() % U8_MAX));
        OS_ListAppend(&os_test_list, item_l_p);
    }
    OS_LOG(D_DEBUG, "Initial list:");
    TestListLog(&os_test_list);
    // Sort the list.
    TestListSort(&os_test_list, SORT_ASCENDING);
    OS_LOG(D_DEBUG, "List sorted by ascending:");
    TestListLog(&os_test_list);
    TestListSort(&os_test_list, SORT_DESCENDING);
    OS_LOG(D_DEBUG, "List sorted by descending:");
    TestListLog(&os_test_list);
    OS_TRACE(D_DEBUG, "\n", OS_NULL);
    // Clean up.
    item_l_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(&os_test_list));
    while (OS_TRUE != OS_LIST_IS_EMPTY(&os_test_list)) {
        item_next_p = OS_LIST_ITEM_NEXT_GET(item_l_p);
        OS_ListItemDelete(item_l_p);
        item_l_p = item_next_p;
    }
}

/******************************************************************************/
void TestListSort(const OS_List* list_p, const SortDirection sort_dir)
{
OS_ListItem* item_min_p;
OS_ListItem* item_curr_p;
OS_ListItem* item_next_p;

    if (OS_NULL == list_p) { return; }
    item_curr_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(list_p));
    while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(item_curr_p)) {
        item_min_p = item_curr_p;
        item_next_p = OS_LIST_ITEM_NEXT_GET(item_curr_p);
        while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(item_next_p)) {
            if (SORT_ASCENDING == sort_dir) {
                if (OS_LIST_ITEM_VALUE_GET(item_next_p) < OS_LIST_ITEM_VALUE_GET(item_min_p)) {
                    item_min_p = item_next_p;
                }
            } else if (SORT_DESCENDING == sort_dir) {
                if (OS_LIST_ITEM_VALUE_GET(item_next_p) > OS_LIST_ITEM_VALUE_GET(item_min_p)) {
                    item_min_p = item_next_p;
                }
            } else { OS_ASSERT(OS_FALSE); }
            item_next_p = OS_LIST_ITEM_NEXT_GET(item_next_p);
        }
        if (item_curr_p != item_min_p) {
            OS_ListItemsSwap(item_curr_p, item_min_p);
            //TestListLog(list_p);
        }
        item_curr_p = OS_LIST_ITEM_NEXT_GET(item_min_p);
    }
}

/******************************************************************************/
void TestListLog(const OS_List* list_p)
{
OS_ListItem* item_l_p;

    if (OS_NULL == list_p) { return; }
    // Trace the results.
    OS_TRACE(D_DEBUG, "\n", OS_NULL);
    item_l_p = OS_LIST_ITEM_NEXT_GET((OS_ListItem*)&OS_LIST_ITEM_LAST_GET(list_p));
    while (OS_DELAY_MAX != OS_LIST_ITEM_VALUE_GET(item_l_p)) {
        OS_TRACE(D_DEBUG, "%d ", (int)OS_LIST_ITEM_VALUE_GET(item_l_p));
        item_l_p = OS_LIST_ITEM_NEXT_GET(item_l_p);
    }
}

#endif // TEST
