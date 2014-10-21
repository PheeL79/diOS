/***************************************************************************//**
* @file    os_usb.h
* @brief   OS USB.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_USB_H_
#define _OS_USB_H_

#include "os_common.h"
#include "os_signal.h"
#include "os_message.h"

/**
* \defgroup OS_Usb OS_Usb
* @{
*/
//------------------------------------------------------------------------------
#define OS_USB_SIG_ITF_GET(itf)         BF_GET(itf, 0,  BIT_SIZE(U8))
#define OS_USB_SIG_MSG_GET(msg)         BF_GET(msg, 8,  BIT_SIZE(U8))

#define OS_USB_SIG_ITF_SET(sig, itf)    BF_SET(sig, itf, 0, BIT_SIZE(U8));
#define OS_USB_SIG_MSG_SET(sig, msg)    BF_SET(sig, msg, 8, BIT_SIZE(U8));

enum {
    DRV_ID_USBH,
    DRV_ID_USBH_FS_MSC,
    DRV_ID_USBH_HS_MSC,
    DRV_ID_USBD,
    DRV_ID_USBD_FS_MSC,
    DRV_ID_USBD_HS_MSC,
    DRV_ID_USBX_LAST
};

enum {
//USB Common
    OS_SIG_USB_CONNECT = OS_SIG_APP,
    OS_SIG_USB_DISCONNECT,
//USB Host
    OS_SIG_USBH_EVENT_PORT,
    OS_SIG_USBH_EVENT_STATE_CHANGED,
    OS_SIG_USBH_EVENT_CONTROL,
    OS_SIG_USBH_EVENT_CLASS,
    OS_SIG_USBH_EVENT_URB,
//USB Class
    OS_SIG_USB_HID_MOUSE,
    OS_SIG_USB_HID_KEYBOARD,

    OS_SIG_USB_LAST
};

enum {
//USB Common
    OS_MSG_USB_CONNECT = OS_MSG_APP,
    OS_MSG_USB_DISCONNECT,
//USB Class
    OS_MSG_USB_HID_MOUSE,
    OS_MSG_USB_HID_KEYBOARD
};

typedef enum {
    OS_USB_ID_UNDEF,
    OS_USB_ID_FS,
    OS_USB_ID_HS,
    OS_USB_ID_LAST
} OS_UsbItfId;

//http://www.usb.org/developers/defined_class
typedef enum {
    OS_USB_CLASS_UNDEF,
    OS_USB_CLASS_AUDIO  = 0x01,
    OS_USB_CLASS_HID    = 0x03,
    OS_USB_CLASS_MTP    = 0x06,
    OS_USB_CLASS_MSC    = 0x08,
    OS_USB_CLASS_CDC    = 0x0A,
    OS_USB_CLASS_LAST
} OS_UsbClass;

typedef enum {
    OS_USB_HID_MOUSE_BUTTON_LEFT,
    OS_USB_HID_MOUSE_BUTTON_RIGHT,
    OS_USB_HID_MOUSE_BUTTON_MIDDLE
} OS_UsbHidMouseButton;

typedef enum {
    OS_USB_HID_KEY_LEFT_CTRL,
    OS_USB_HID_KEY_LEFT_SHIFT,
    OS_USB_HID_KEY_LEFT_ALT,
    OS_USB_HID_KEY_LEFT_GUI,
    OS_USB_HID_KEY_RIGHT_CTRL,
    OS_USB_HID_KEY_RIGHT_SHIFT,
    OS_USB_HID_KEY_RIGHT_ALT,
    OS_USB_HID_KEY_RIGHT_GUI
} OS_UsbHidKeyboardModKey;

typedef const void* OS_UsbItfHd;

typedef struct {
    OS_UsbItfHd itf_fs_hd;
    OS_UsbItfHd itf_hs_hd;
} OS_UsbHItfHd, OS_UsbDItfHd;

typedef struct {
    OS_UsbItfHd itf_hd;
    OS_UsbItfId itf_id;
    OS_UsbClass class;
} OS_UsbEventData;

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