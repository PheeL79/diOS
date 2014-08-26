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

#endif // TEST
