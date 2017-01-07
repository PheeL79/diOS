/***************************************************************************//**
* @file    test_mock.h
* @brief   Mock driver for tests.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _TEST_MOCK_H_
#define _TEST_MOCK_H_

/** \addtogroup Application
*@{*/
/** \addtogroup Test
*@{*/
/** \defgroup   test_mock Mock
*@{*/
//------------------------------------------------------------------------------
#define HAL_TEST_MOCK_ITF           (HAL_Itf)UINT_MAX   ///< Mocking interface ID

//------------------------------------------------------------------------------
/// @brief      Read data from mock driver.
/// @param[in]  data_in_p       Data input
/// @param[in]  size            Data input size
/// @param[in]  args_p          Driver specific arguments
/// @return     #Status
Status          TEST_MockRead(void* data_in_p, Size size, void* args_p);

/// @brief      Write data into mock driver.
/// @param[in]  data_out_p      Data output
/// @param[in]  size            Data output size
/// @param[in]  args_p          Driver specific arguments
/// @return     #Status
Status          TEST_MockWrite(void* data_out_p, Size size, void* args_p);

/**@}*/ //test_mock
/**@}*/ //Test
/**@}*/ //Application

#endif //_TEST_MOCK_H_
