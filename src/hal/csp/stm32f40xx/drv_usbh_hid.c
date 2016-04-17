/**************************************************************************//**
* @file    drv_usbh_hid.c
* @brief   USB Host HID driver.
* @author  A. Filyanov
******************************************************************************/
#include "os_config.h"

#if (HAL_USBH_ENABLED) && (HAL_USBH_HID_ENABLED)

#include "usbh_hid.h"
#include "os_common.h"
#include "os_debug.h"

//-----------------------------------------------------------------------------
#define MDL_NAME            "drv_usb_hid"

/******************************************************************************/
static void MouseDataTranslate(const HID_MOUSE_Info_TypeDef* mouse_info_p, OS_UsbHidMouseData* mouse_data_p);
INLINE void MouseDataTranslate(const HID_MOUSE_Info_TypeDef* mouse_info_p, OS_UsbHidMouseData* mouse_data_p)
{
    mouse_data_p->x = mouse_info_p->x;
    mouse_data_p->y = mouse_info_p->y;
    mouse_data_p->buttons_bf = BIT_SHIFT(BIT_TEST(mouse_info_p->buttons[0], 1), OS_USB_HID_MOUSE_BUTTON_LEFT);
    mouse_data_p->buttons_bf|= BIT_SHIFT(BIT_TEST(mouse_info_p->buttons[1], 1), OS_USB_HID_MOUSE_BUTTON_RIGHT);
    mouse_data_p->buttons_bf|= BIT_SHIFT(BIT_TEST(mouse_info_p->buttons[2], 1), OS_USB_HID_MOUSE_BUTTON_MIDDLE);
}

/******************************************************************************/
static void KeyboardDataTranslate(HID_KEYBD_Info_TypeDef* keyboard_info_p, OS_UsbHidKeyboardData* keyboard_data_p);
INLINE void KeyboardDataTranslate(HID_KEYBD_Info_TypeDef* keyboard_info_p, OS_UsbHidKeyboardData* keyboard_data_p)
{
    keyboard_data_p->keys_func_bf  = BIT_SHIFT(BIT_TEST(keyboard_info_p->lctrl, 1), OS_USB_HID_KEY_LEFT_CTRL);
    keyboard_data_p->keys_func_bf |= BIT_SHIFT(BIT_TEST(keyboard_info_p->lshift, 1),OS_USB_HID_KEY_LEFT_SHIFT);
    keyboard_data_p->keys_func_bf |= BIT_SHIFT(BIT_TEST(keyboard_info_p->lalt, 1),  OS_USB_HID_KEY_LEFT_ALT);
    keyboard_data_p->keys_func_bf |= BIT_SHIFT(BIT_TEST(keyboard_info_p->lgui, 1),  OS_USB_HID_KEY_LEFT_GUI);
    keyboard_data_p->keys_func_bf |= BIT_SHIFT(BIT_TEST(keyboard_info_p->rctrl, 1), OS_USB_HID_KEY_RIGHT_CTRL);
    keyboard_data_p->keys_func_bf |= BIT_SHIFT(BIT_TEST(keyboard_info_p->rshift, 1),OS_USB_HID_KEY_RIGHT_SHIFT);
    keyboard_data_p->keys_func_bf |= BIT_SHIFT(BIT_TEST(keyboard_info_p->ralt, 1),  OS_USB_HID_KEY_RIGHT_ALT);
    keyboard_data_p->keys_func_bf |= BIT_SHIFT(BIT_TEST(keyboard_info_p->rgui, 1),  OS_USB_HID_KEY_RIGHT_GUI);
    keyboard_data_p->key_state     = keyboard_info_p->state;
    keyboard_data_p->key_ascii     = USBH_HID_GetASCIICode(keyboard_info_p);
    OS_MemCpy(keyboard_data_p->keys_raw_v, keyboard_info_p->keys, sizeof(keyboard_data_p->keys_raw_v));
}

/******************************************************************************/
/**
  * @brief  The function is a callback about HID Data events
  * @param  phost: Selected device
  * @retval None
  */
void USBH_HID_EventCallback(USBH_HandleTypeDef *phost)
{
const HID_TypeTypeDef hid_type = USBH_HID_GetDeviceType(phost);
OS_Message* msg_p;
    if (HID_MOUSE == hid_type) {
        const HID_MOUSE_Info_TypeDef* mouse_info_p = USBH_HID_GetMouseInfo(phost);
        OS_UsbHidMouseData mouse_data;
        OS_ASSERT_DEBUG(NULL != mouse_info_p);
        MouseDataTranslate(mouse_info_p, &mouse_data);
        msg_p = OS_MessageCreate(OS_MSG_USB_HID_MOUSE, &mouse_data, sizeof(mouse_data), OS_TIMEOUT_DRIVER);
    } else if (HID_KEYBOARD == hid_type) {
        HID_KEYBD_Info_TypeDef* keyboard_info_p = USBH_HID_GetKeybdInfo(phost);
        OS_UsbHidKeyboardData keyboard_data;
        OS_ASSERT_DEBUG(NULL != keyboard_info_p);
        KeyboardDataTranslate(keyboard_info_p, &keyboard_data);
        msg_p = OS_MessageCreate(OS_MSG_USB_HID_KEYBOARD, &keyboard_data, sizeof(keyboard_data), OS_TIMEOUT_DRIVER);
    } else {
        OS_LOG(L_WARNING, "Unknown HID type!");
        OS_ASSERT(OS_FALSE);
    }
    OS_ASSERT_DEBUG(OS_NULL != msg_p);
    OS_ASSERT(S_OK == OS_MessageEmit(msg_p, OS_TIMEOUT_DRIVER, OS_MSG_PRIO_NORMAL));
}

#endif //(HAL_USBH_ENABLED) && (HAL_USBH_HID_ENABLED)
