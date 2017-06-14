#include "hal.h"
#include "usbh.h"

#if DRIVER_USBH_USE_DS4

#if !DRIVER_USE_USBH
#error "USBHDS4 needs USBH"
#endif

#include "drivers_conf.h"

#include <string.h>
#include "usbh/dev/ds4.h"
#include "usbh/internal.h"

#include "ds4.h"

#if USBHDS4_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
//#define udbgf(f, ...)  do {} while(0)
//#define udbg(f, ...)   do {} while(0)
#endif

#if USBHDS4_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
//#define uinfof(f, ...)  do {} while(0)
//#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHDS4_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
//#define uwarnf(f, ...)  do {} while(0)
//#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHDS4_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
//#define uerrf(f, ...)  do {} while(0)
//#define uerr(f, ...)   do {} while(0)
#endif

#define INT_POLL_INTERVAL_MS 200

USBHDS4Driver USBHDS4[DRIVER_USBHDS4_MAX_INSTANCES];

static usbh_baseclassdriver_t *_ds4_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _ds4_unload(usbh_baseclassdriver_t *drv);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	_ds4_load,
	_ds4_unload
};

const usbh_classdriverinfo_t usbhds4ClassDriverInfo = {
	0x03, 0x00, 0x00, "Dual Shock 4", &class_driver_vmt
};

static usbh_baseclassdriver_t *_ds4_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {
	int i;
	USBHDS4Driver *ds4p;

	if (dev->devDesc.idVendor != 0x054C) {
    //uerr("Wrong VID");
    return NULL;
  }

	if (dev->devDesc.idProduct != 0x05C4 && dev->devDesc.idProduct != 0x09CC) {//gen2 controller
   // uerr("Wrong PID");
    return NULL;
  }

	if ((rem < descriptor[0]) || (descriptor[1] != USBH_DT_INTERFACE))
    return NULL;

	/* alloc driver */
	for (i = 0; i < DRIVER_USBHDS4_MAX_INSTANCES; i++) {
		if (USBHDS4[i].dev == NULL) {
			ds4p = &USBHDS4[i];
			goto alloc_ok;
		}
	}

	//uwarn("Can't alloc DS4 driver");

	/* can't alloc */
	return NULL;

alloc_ok:
	/* initialize the driver's variables */
	ds4p->epin.status = USBH_EPSTATUS_UNINITIALIZED;
	ds4p->epout.status = USBH_EPSTATUS_UNINITIALIZED;

	usbhEPSetName(&dev->ctrl, "DS4[CTRL]");

	/* parse the configuration descriptor */
	if_iterator_t iif;
	generic_iterator_t iep;
	iif.iad = 0;
	iif.curr = descriptor;
	iif.rem = rem;
	for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
		const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);
		if ((epdesc->bEndpointAddress & 0x80) && (epdesc->bmAttributes == USBH_EPTYPE_INT)) {
			//uinfof("INT IN endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&ds4p->epin, dev, epdesc);
			usbhEPSetName(&ds4p->epin, "DS4[INTIN]");
		} else if (((epdesc->bEndpointAddress & 0x80) == 0)
				&& (epdesc->bmAttributes == USBH_EPTYPE_INT)) {
			//uinfof("INT OUT endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&ds4p->epout, dev, epdesc);
			usbhEPSetName(&ds4p->epout, "DS4[INTOUT]");
		} else {
			//uinfof("unsupported endpoint found: bEndpointAddress=%02x, bmAttributes=%02x",
			//		epdesc->bEndpointAddress, epdesc->bmAttributes);
		}
	}
	if ((ds4p->epin.status != USBH_EPSTATUS_CLOSED) || (ds4p->epout.status != USBH_EPSTATUS_CLOSED)) {
		goto deinit;
	}

	ds4p->state = USBHDS4_STATE_ACTIVE;

	return (usbh_baseclassdriver_t *)ds4p;

deinit:
	/* Here, the enpoints are closed, and the driver is unlinked */
	return NULL;
}

static void _ds4_unload(usbh_baseclassdriver_t *drv) {
	osalDbgCheck(drv != NULL);
	USBHDS4Driver *const ds4p = (USBHDS4Driver *)drv;
	usbhds4Stop(ds4p);
}

static usbh_urbstatus_t _setidle(USBHDS4Driver *ds4p, uint8_t duration, uint8_t reportId) {

  static const uint8_t bmRequestType = \
      USBH_REQTYPE_CLASS | USBH_REQTYPE_OUT | USBH_REQTYPE_INTERFACE;

  static const uint8_t bRequest = USB_HID_SET_IDLE;

  uint16_t wValue = (duration << 8 ) | reportId;

  static const uint16_t wIndex = 0;

  static const uint16_t wLength = 0;

  return usbhControlRequest(ds4p->dev, bmRequestType, bRequest, wValue, wIndex, wLength, NULL);
}

static usbh_urbstatus_t _getreportdescriptor(USBHDS4Driver *ds4p, uint16_t length, uint8_t *buff){

  static const uint8_t bmRequestType = \
      USBH_REQTYPE_STANDARD | USBH_REQTYPE_IN | USBH_REQTYPE_INTERFACE;

  static const uint8_t bRequest = USBH_REQ_GET_DESCRIPTOR;

  static const uint16_t wValue = USB_DESC_HID_REPORT;

  static const uint16_t wIndex = 0;

  uint16_t wLength = length;

  return usbhControlRequest(ds4p->dev, bmRequestType, bRequest, wValue, wIndex, wLength, buff);
}

static void _in_cb(usbh_urb_t *urb) {
  USBHDS4Driver *const ds4p = (USBHDS4Driver *)(urb->userData);
  if(urb->status == USBH_URBSTATUS_OK){
    ds4p->packet_handler((uint8_t *)urb->buff, urb->actualLength);
    usbhURBObjectResetI(&(ds4p->in_urb));
    usbhURBSubmitI(&(ds4p->in_urb));
  }
}




static void _vt(void *p) {
  USBHDS4Driver *const ds4p = (USBHDS4Driver *)p;
  chSysLockFromISR();
  if (ds4p->in_urb.status != USBH_URBSTATUS_OK && ds4p->in_urb.status != USBH_URBSTATUS_PENDING){
    usbhURBObjectResetI(&(ds4p->in_urb));
    usbhURBSubmitI(&(ds4p->in_urb));
  }
  chVTSetI(&ds4p->vt, MS2ST(INT_POLL_INTERVAL_MS), _vt, ds4p);
  chSysUnlockFromISR();
}

void usbhds4Start(USBHDS4Driver *ds4p){

  osalDbgCheck((ds4p->state == USBHDS4_STATE_ACTIVE)
      || (ds4p->state == USBHDS4_STATE_READY));

  if (ds4p->state == USBHDS4_STATE_READY)
    return;

  /* HID Class request */
  usbh_urbstatus_t stat;
  // Set Idle
  usbhEPClose(&ds4p->dev->ctrl);
  usbhEPOpen(&ds4p->dev->ctrl);
  stat = _setidle(ds4p, 0, 0);
  if (stat == USBH_URBSTATUS_OK) {
  } else if (stat == USBH_URBSTATUS_STALL) {
   // uwarn("Set Idle Stall");
  } else {
   // uerr("Set Idle Error");
  }

  // Get report descriptor
  USBH_DEFINE_BUFFER(uint8_t, buff[483]);
  stat = _getreportdescriptor(ds4p, 483, buff);
  if (stat == USBH_URBSTATUS_OK) {
  } else if (stat == USBH_URBSTATUS_STALL) {
   // uwarn("GetHIDReportDescriptor Stall");
  } else {
   // uerr("GetHIDReportDescriptor Error");
  }

  /* open the INT IN/OUT endpoints */
  usbhURBObjectInit(&ds4p->out_urb, &ds4p->epout, NULL, ds4p, ds4p->out_buff, 0);
  usbhEPOpen(&ds4p->epout);

  usbhURBObjectInit(&ds4p->in_urb, &ds4p->epin, _in_cb, ds4p, ds4p->in_buff, 64);
  usbhEPOpen(&ds4p->epin);
  osalSysLock();
  usbhURBSubmitI(&ds4p->in_urb);
  osalSysUnlock();

  chVTObjectInit(&ds4p->vt);
  chVTSet(&ds4p->vt, MS2ST(INT_POLL_INTERVAL_MS), _vt, ds4p);

  ds4p->state = USBHDS4_STATE_READY;

}

void usbhds4Stop(USBHDS4Driver *ds4p){
  osalDbgCheck((ds4p->state == USBHDS4_STATE_ACTIVE)
      || (ds4p->state == USBHDS4_STATE_READY));

  if (ds4p->state == USBHDS4_STATE_ACTIVE) {
    return;
  }

  osalSysLock();
  chVTResetI(&ds4p->vt);
  usbhEPCloseS(&ds4p->epin);
  usbhEPCloseS(&ds4p->epout);
  osalOsRescheduleS();
  ds4p->state = USBHDS4_STATE_STOP;
  osalSysUnlock();
}

int usbhds4Send(USBHDS4Driver *ds4p, uint8_t *packet){
  if (ds4p->state != USBHDS4_STATE_READY) return -1;

  // prepare transfer
  ds4p->out_buff[0] = 0x05;
  ds4p->out_buff[1] = 0xFF;
  memcpy(&(ds4p->out_buff[4]), packet, 7);
  ds4p->out_urb.requestedLength = 32;

  // submit transfer
  osalSysLock();
  usbhURBObjectResetI(&(ds4p->out_urb));
  usbhURBSubmitI(&(ds4p->out_urb));
  osalSysUnlock();

  return 0;
}

void usbhds4ObjectInit(USBHDS4Driver *ds4p) {
  osalDbgCheck(ds4p != NULL);
  memset(ds4p, 0, sizeof(*ds4p));
  ds4p->info = &usbhds4ClassDriverInfo;
}

#endif
