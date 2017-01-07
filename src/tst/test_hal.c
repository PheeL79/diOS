//-----------------------------------------------------------------------------
#include "hal_config.h"

#ifdef HAL_TEST_ENABLED
#include <stdlib.h>
#include "unity.h"
#include "common.h"
#include "hal.h"
#include "test_hal.h"

//-----------------------------------------------------------------------------
extern void setUp(void);
extern void tearDown(void);

//-----------------------------------------------------------------------------
static void runTest(UnityTestFunction test);
void resetTest(void);

/******************************************************************************/
void TestsHalRun(void)
{
    UnityBegin("test_hal.c");
//    RUN_TEST(TestList, 1);
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

#endif //HAL_TEST_ENABLED
