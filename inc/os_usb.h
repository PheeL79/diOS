/***************************************************************************//**
* @file    os_usb.h
* @brief   OS USB.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_USB_H_
#define _OS_USB_H_

#include "os_common.h"

/**
* \defgroup OS_Usb OS_Usb
* @{
*/
//------------------------------------------------------------------------------
enum {
//USB Common
    OS_SIG_USB_CONNECT = OS_SIG_APP,
    OS_SIG_USB_DISCONNECT,
    OS_SIG_USB_READY,
//USB Host
    OS_SIG_USBH_PORT_EVENT,
    OS_SIG_USBH_STATE_CHANGED_EVENT,
    OS_SIG_USBH_CONTROL_EVENT,
    OS_SIG_USBH_CLASS_EVENT,
    OS_SIG_USBH_URB_EVENT,
//USB Device
    OS_SIG_USB_LAST
};

//http://www.usb.org/developers/defined_class
enum {
    OS_USB_CLASS_UNDEF,
    OS_USB_CLASS_AUDIO  = 0x01,
    OS_USB_CLASS_HID    = 0x03,
    OS_USB_CLASS_MTP    = 0x06,
    OS_USB_CLASS_MSC    = 0x08,
    OS_USB_CLASS_CDC    = 0x0A,
    OS_USB_CLASS_LAST
};

/**@}*/ //OS_Usb

#endif // _OS_USB_H_