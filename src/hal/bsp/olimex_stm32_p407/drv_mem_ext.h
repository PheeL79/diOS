/**************************************************************************//**
* @file    drv_mem_ext.h
* @brief   External memory driver.
* @author  Aleksandar Mitev
******************************************************************************/
#ifndef _DRV_MEM_EXT_H_
#define _DRV_MEM_EXT_H_

//-----------------------------------------------------------------------------
enum {
    DRV_ID_MEM_EXT_SRAM512K,
    DRV_ID_MEM_EXT_LAST
};

//-----------------------------------------------------------------------------
extern HAL_DriverItf* drv_mem_ext_v[];

//-----------------------------------------------------------------------------
Status MEM_EXT_Test(void); //TODO(A.Filyanov) Move to IoCtl driver function!

#endif // _DRV_MEM_EXT_H_