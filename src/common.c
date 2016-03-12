/***************************************************************************//**
* @file    common.c
* @brief   Common.
* @author  A. Filyanov
*******************************************************************************/
#include "common.h"

/// @author https://graphics.stanford.edu/~seander/bithacks.html
/******************************************************************************/
UF8 Bit2PositionGet(const U32 value)
{
static const UF8 MultiplyDeBruijnBitPosition2[32] =
{
  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};
    return MultiplyDeBruijnBitPosition2[(UF8)(value * 0x077CB531U) >> 27];
}