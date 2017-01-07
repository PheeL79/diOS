//-----------------------------------------------------------------------------
#include "hal.h"

#if (HAL_TEST_ENABLED)
#include <string.h>
#include "test_mock.h"
#include "command.h"

//------------------------------------------------------------------------------
ALIGN_BEGIN static U8 test_mock_cmd_buf_v[PROTOCOL_PACKET_SIZE_MAX] ALIGN_END;
static Size pkt_size = 0;

/******************************************************************************/
Status TEST_MockRead(void* data_in_p, Size size, void* args_p)
{
Status s = S_OK;
    (void)size;
    U8** data8_in_pp= (U8**)data_in_p;
    Size* size_p    = (Size*)args_p;
    *data8_in_pp    = &test_mock_cmd_buf_v[0];
    *size_p         = pkt_size;
    return s;
}

/******************************************************************************/
Status TEST_MockWrite(void* data_out_p, Size size, void* args_p)
{
Status s = S_OK;
    (void)args_p;
    pkt_size = size;
    HAL_MemCpy(&test_mock_cmd_buf_v[0], data_out_p, pkt_size);
    return s;
}

#endif //(HAL_TEST_ENABLED)
