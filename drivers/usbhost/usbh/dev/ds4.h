#ifndef USBH_DS4_H_
#define USBH_DS4_H_

#include "usbh.h"

#if DRIVER_USE_USBH && DRIVER_USBH_USE_DS4

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
#define USB_HID_GET_REPORT           0x01
#define USB_HID_GET_IDLE             0x02
#define USB_HID_GET_PROTOCOL         0x03
#define USB_HID_SET_REPORT           0x09
#define USB_HID_SET_IDLE             0x0A
#define USB_HID_SET_PROTOCOL         0x0B

#define USB_DESC_TYPE_HID            0x21
#define USB_DESC_TYPE_HID_REPORT     0x22
#define USB_HID_DESC_SIZE            9

#define USB_DESC_HID_REPORT          ((USB_DESC_TYPE_HID_REPORT << 8) & 0xFF00)
#define USB_DESC_HID                 ((USB_DESC_TYPE_HID << 8) & 0xFF00)


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
typedef enum {
  USBHDS4_STATE_UNINIT = 0,
  USBHDS4_STATE_STOP = 1,
  USBHDS4_STATE_ACTIVE = 2,
  USBHDS4_STATE_READY = 3
} usbhds4_state_e;

typedef struct {
  /* inherited from abstract class driver */
  _usbh_base_classdriver_data

  usbhds4_state_e state;

  usbh_ep_t epin;
  usbh_urb_t in_urb;
  USBH_DEFINE_BUFFER(uint8_t, in_buff[64]);

  usbh_ep_t epout;
  usbh_urb_t out_urb;
  USBH_DEFINE_BUFFER(uint8_t, out_buff[64]);

  void (*packet_handler)(uint8_t *packet, uint16_t size);

  virtual_timer_t vt;
} USBHDS4Driver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
extern USBHDS4Driver USBHDS4[DRIVER_USBHDS4_MAX_INSTANCES];

#ifdef __cplusplus
extern "C" {
#endif
  void usbhds4ObjectInit(USBHDS4Driver *ds4p);
  void usbhds4Start(USBHDS4Driver *ds4p);
  void usbhds4Stop(USBHDS4Driver *ds4p);
  int usbhds4Send(USBHDS4Driver *ds4p, uint8_t *packet);
#ifdef __cplusplus
}
#endif


#endif

#endif /* USBH_DS4_H_ */
