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
#define OS_USBH_SIG_ITF_GET(itf)        BF_GET(itf, 0,  BIT_SIZE(U8))
#define OS_USBH_SIG_MSG_GET(msg)        BF_GET(msg, 8,  BIT_SIZE(U8))

#define OS_USBH_SIG_ITF_SET(sig, itf)   BF_SET(sig, itf, 0, BIT_SIZE(U8));
#define OS_USBH_SIG_MSG_SET(sig, msg)   BF_SET(sig, msg, 8, BIT_SIZE(U8));

enum {
    USBH_ID_FS,
    USBH_ID_HS
};

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
//USB Class
    OS_SIG_USB_HID_MOUSE,
    OS_SIG_USB_HID_KEYBOARD,

    OS_SIG_USB_LAST
};

enum {
    OS_MSG_USB_HID_MOUSE = OS_MSG_APP,
    OS_MSG_USB_HID_KEYBOARD
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

enum {
    OS_USB_HID_MOUSE_BUTTON_LEFT,
    OS_USB_HID_MOUSE_BUTTON_RIGHT,
    OS_USB_HID_MOUSE_BUTTON_MIDDLE
};

enum {
    OS_USB_HID_KEY_LEFT_CTRL,
    OS_USB_HID_KEY_LEFT_SHIFT,
    OS_USB_HID_KEY_LEFT_ALT,
    OS_USB_HID_KEY_LEFT_GUI,
    OS_USB_HID_KEY_RIGHT_CTRL,
    OS_USB_HID_KEY_RIGHT_SHIFT,
    OS_USB_HID_KEY_RIGHT_ALT,
    OS_USB_HID_KEY_RIGHT_GUI
};

typedef const void* OS_UsbHd;

typedef struct {
    OS_UsbHd usbh_fs_hd;
    OS_UsbHd usbh_hs_hd;
} OS_UsbhHd;

typedef struct {
    S16  x;
    S16  y;
    S8   buttons_bm;
} OS_UsbHidMouseData;

typedef struct {
    U8  key_state;
    U8  key_ascii;
    U16 keys_func_bm;
    U8  keys_raw_v[6];
} OS_UsbHidKeyboardData;

/**@}*/ //OS_Usb

#endif // _OS_USB_H_