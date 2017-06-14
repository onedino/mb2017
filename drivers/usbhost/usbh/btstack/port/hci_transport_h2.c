#include "hal.h"
#include "usbh.h"

#if DRIVER_USBH_USE_BTD

#if !DRIVER_USE_USBH
#error "BTD needs USBH"
#endif

#include <string.h>
#include "usbh/internal.h"
#include "btstack_config.h"
#include "btstack_run_loop_embedded.h"
#include "btstack_debug.h"

#include "hci.h"
#include "hci_transport.h"

#if USBHBTD_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBHBTD_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHBTD_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHBTD_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif

#define ASYNC_POLLING_INTERVAL_MS 10
#define URB_MAILBOX_SIZE 8

typedef struct {
  /* inherited from abstract class driver */
  _usbh_base_classdriver_data
  hci_state_e state;
  virtual_timer_t vt;
}hci_driver_t;

static mailbox_t btstack_urb_mb;
static msg_t btstack_urb_queue[URB_MAILBOX_SIZE];

static btstack_data_source_t hci_data_source;

static int usb_acl_out_active = 0;
static int usb_command_active = 0;

static usbh_urb_t hci_ctrl_urb;
static USBH_DEFINE_BUFFER(usbh_control_request_t, req) = {
USBH_REQTYPE_CLASS | USBH_REQTYPE_OUT | USBH_REQTYPE_DEVICE, 0, 0, 0, 0 };

static usbh_ep_t hci_event_in_ep; // EP1, IN interrupt
static usbh_urb_t hci_event_in_urb;
static USBH_DEFINE_BUFFER(uint8_t, hci_event_in_buffer[HCI_ACL_BUFFER_SIZE]);

static usbh_ep_t hci_acl_in_ep; // EP2, IN bulk
static usbh_urb_t hci_acl_in_urb;
static USBH_DEFINE_BUFFER(uint8_t, hci_acl_in_buffer[HCI_ACL_BUFFER_SIZE]);

static usbh_ep_t hci_acl_out_ep; // EP2, OUT bulk
static usbh_urb_t hci_acl_out_urb;

static void dummy_handler(uint8_t packet_type, uint8_t *packet, uint16_t size);

static void (*packet_handler)(uint8_t packet_type, uint8_t *packet, uint16_t size) = dummy_handler;

static hci_driver_t hci_driver;
// single instance
static hci_transport_t hci_transport_usb;

static usbh_baseclassdriver_t *_hci_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _hci_unload(usbh_baseclassdriver_t *drv);
int usb_close(void);

static const usbh_classdriver_vmt_t class_driver_vmt = {
  _hci_load,
  _hci_unload
};

const usbh_classdriverinfo_t usbhbtdClassDriverInfo = {
	0xE0, 0x01, 0x01, "BT HCI", &class_driver_vmt
};

static void handle_completed_transfer(usbh_urb_t *transfer) {

  int resubmit = 0;
  int signal_done = 0;

  if (transfer->ep == &hci_event_in_ep) {
    packet_handler(HCI_EVENT_PACKET, transfer->buff, transfer->actualLength);
    resubmit = 1;
  } else if (transfer->ep == &hci_acl_in_ep) {
    packet_handler(HCI_ACL_DATA_PACKET, transfer->buff, transfer->actualLength);
    resubmit = 1;
  } else if (transfer->ep == &hci_driver.dev->ctrl) {
    usb_command_active = 0;
    signal_done = 1;
  } else if (transfer->ep == &hci_acl_out_ep) {
    usb_acl_out_active = 0;
    signal_done = 1;
  } else {
    uwarnf("usb_process_ds endpoint unknown %x", transfer->ep);
  }

  if (signal_done) {
    // notify upper stack that provided buffer can be used again
    uint8_t event[] = { HCI_EVENT_TRANSPORT_PACKET_SENT, 0 };
    packet_handler(HCI_EVENT_PACKET, &event[0], sizeof(event));
  }

  if (hci_driver.state != HCI_STATE_READY)
    return;

  if (resubmit) {
    // Re-submit transfer

    transfer->userData = NULL;
    osalSysLock();
    usbhURBObjectResetI(transfer);
    usbhURBSubmitI(transfer);
    osalSysUnlock();
  }
}

static void usb_process_ds(btstack_data_source_t *ds, btstack_data_source_callback_type_t callback_type) {
  (void)ds;
  (void)callback_type;


  // Handle any packet in the order that they were received
  usbh_urb_t *urb;
  while(chMBFetch(&btstack_urb_mb, (msg_t *)&urb, TIME_IMMEDIATE) == MSG_OK){
    handle_completed_transfer(urb);
    // handle case where libusb_close might be called by hci packet handler
    if (hci_driver.state != HCI_STATE_READY)
      return;

  }
}

static void _submitI(void *p){
  (void)p;

  osalSysLockFromISR();
  if (hci_event_in_urb.status != USBH_URBSTATUS_OK && hci_event_in_urb.status != USBH_URBSTATUS_PENDING){
    usbhURBObjectResetI(&hci_event_in_urb);
    usbhURBSubmitI(&hci_event_in_urb);
  }
  if (hci_acl_in_urb.status != USBH_URBSTATUS_OK && hci_acl_in_urb.status != USBH_URBSTATUS_PENDING){
    usbhURBObjectResetI(&hci_acl_in_urb);
    usbhURBSubmitI(&hci_acl_in_urb);
  }

  chVTSetI(&hci_driver.vt, MS2ST(ASYNC_POLLING_INTERVAL_MS), _submitI, NULL);
  osalSysUnlockFromISR();
}

// list of known devices, using VendorID/ProductID tuples
static const uint16_t known_bt_devices[] = {
  // CSR8510 HCI mode
  0x0A12, 0x0001,
};

static int num_known_devices = sizeof(known_bt_devices) / sizeof(uint16_t) / 2;

static int is_known_bt_device(uint16_t vendor_id, uint16_t product_id) {
  int i;
  for (i = 0; i < num_known_devices; i++) {
    if (known_bt_devices[i * 2] == vendor_id && known_bt_devices[i * 2 + 1] == product_id) {
      return 1;
    }
  }
  return 0;
}

static usbh_baseclassdriver_t *_hci_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {

	// Detect USB Dongle based on whitelist
  if (!is_known_bt_device(dev->devDesc.idVendor, dev->devDesc.idProduct)) {
    return NULL;
  }

	if ((rem < descriptor[0]) || (descriptor[1] != USBH_DT_DEVICE))
    return NULL;

  generic_iterator_t iep, icfg;
  if_iterator_t iif;

  cfg_iter_init(&icfg, dev->fullConfigurationDescriptor,
      dev->basicConfigDesc.wTotalLength);

  if_iter_init(&iif, &icfg);
  if (!iif.valid)
    return NULL;
  const usbh_interface_descriptor_t * const ifdesc = if_get(&iif);
  if ((ifdesc->bInterfaceClass != 0xe0)
   || (ifdesc->bInterfaceSubClass != 0x01)
   || (ifdesc->bInterfaceProtocol != 0x01)) {
    return NULL;
  }

	/* initialize the driver's variables */
	hci_event_in_ep.status = USBH_EPSTATUS_UNINITIALIZED;
	hci_acl_in_ep.status = USBH_EPSTATUS_UNINITIALIZED;
	hci_acl_out_ep.status = USBH_EPSTATUS_UNINITIALIZED;

	usbhEPSetName(&dev->ctrl, "HCI command");

	/* parse the configuration descriptor */
	for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
		const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);
		if ((epdesc->bEndpointAddress & 0x80) && (epdesc->bmAttributes == USBH_EPTYPE_INT)) {
			uinfof("INT IN endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&hci_event_in_ep, dev, epdesc);
			usbhEPSetName(&hci_event_in_ep, "HCI Event IN");
		} else if ((epdesc->bEndpointAddress & 0x80) && (epdesc->bmAttributes == USBH_EPTYPE_BULK)) {
      uinfof("BULK IN endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
      usbhEPObjectInit(&hci_acl_in_ep, dev, epdesc);
      usbhEPSetName(&hci_acl_in_ep, "HCI ACL IN");
		} else if (((epdesc->bEndpointAddress & 0x80) == 0) && (epdesc->bmAttributes == USBH_EPTYPE_BULK)) {
			uinfof("BULK OUT endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&hci_acl_out_ep, dev, epdesc);
			usbhEPSetName(&hci_acl_out_ep, "HCI ACL OUT");
		} else {
			uinfof("unsupported endpoint found: bEndpointAddress=%02x, bmAttributes=%02x",
					epdesc->bEndpointAddress, epdesc->bmAttributes);
		}
	}

	if ((hci_event_in_ep.status != USBH_EPSTATUS_CLOSED) ||
	    (hci_acl_in_ep.status != USBH_EPSTATUS_CLOSED) ||
	    (hci_acl_out_ep.status != USBH_EPSTATUS_CLOSED)) {
		return NULL;
	}

	hci_driver.state = HCI_STATE_ACTIVE;

	return (usbh_baseclassdriver_t *)&hci_driver;
}

static void _hci_unload(usbh_baseclassdriver_t *drv) {
	osalDbgCheck(drv != NULL);
	usb_close();
}

static void async_callback(usbh_urb_t *urb) {
  if (hci_driver.state != HCI_STATE_READY)
    return;

  if (urb->status == USBH_URBSTATUS_OK) {
    chMBPostI(&btstack_urb_mb, (msg_t)urb);
    btstack_run_loop_embedded_trigger(1);
  } else if (urb->status == USBH_URBSTATUS_DISCONNECTED) {
    uwarn("HCI: URB disconnected");
  } else if (urb->status == USBH_URBSTATUS_TIMEOUT) {
  } else {
    uerrf("HCI: URB OUT status unexpected = %d", urb->status);
  }
}


int usb_open(void){

  osalDbgCheck((hci_driver.state == HCI_STATE_ACTIVE)
      || (hci_driver.state == HCI_STATE_READY));

  if (hci_driver.state == HCI_STATE_READY)
    return -1;

  // configure event_in handlers

  //CTRL EP
  usbhEPClose(&hci_driver.dev->ctrl);
  usbhEPOpen(&hci_driver.dev->ctrl);
  usbhURBObjectInit(&hci_ctrl_urb, &hci_driver.dev->ctrl, async_callback, 0, NULL, 0);
  hci_ctrl_urb.setup_buff = &req;

  //INT IN EP
  usbhURBObjectInit(&hci_event_in_urb, &hci_event_in_ep,
      async_callback, &hci_driver, &hci_event_in_buffer, HCI_ACL_BUFFER_SIZE);
  usbhEPOpen(&hci_event_in_ep);
  osalSysLock();
  usbhURBObjectResetI(&hci_event_in_urb);
  usbhURBSubmitI(&hci_event_in_urb);
  osalSysUnlock();

  //BULK OUT EP
  usbhEPOpen(&hci_acl_out_ep);
  usbhURBObjectInit(&hci_acl_out_urb, &hci_acl_out_ep,
      async_callback, &hci_driver, NULL, 0);

  //BULK IN EP
  usbhURBObjectInit(&hci_acl_in_urb, &hci_acl_in_ep,
      async_callback, &hci_driver, &hci_acl_in_buffer, HCI_ACL_BUFFER_SIZE);
  usbhEPOpen(&hci_acl_in_ep);
  osalSysLock();
  usbhURBObjectResetI(&hci_acl_in_urb);
  usbhURBSubmitI(&hci_acl_in_urb);
  osalSysUnlock();


  btstack_run_loop_set_data_source_handler(&hci_data_source, &usb_process_ds);
  btstack_run_loop_enable_data_source_callbacks(&hci_data_source, DATA_SOURCE_CALLBACK_READ);
  btstack_run_loop_add_data_source(&hci_data_source);

  chVTObjectInit(&hci_driver.vt);
  chVTSet(&hci_driver.vt, MS2ST(ASYNC_POLLING_INTERVAL_MS), _submitI, NULL);
  chMBObjectInit(&btstack_urb_mb, btstack_urb_queue, URB_MAILBOX_SIZE);

  hci_driver.state = HCI_STATE_READY;

  return 0;
}


int usb_close(void){
  osalDbgCheck((hci_driver.state == HCI_STATE_ACTIVE)
      || (hci_driver.state == HCI_STATE_READY));

  if (hci_driver.state < HCI_STATE_ACTIVE) {
    return -1;
  }

  osalSysLock();
  chVTResetI(&hci_driver.vt);
  chMBResetI(&btstack_urb_mb);
  btstack_run_loop_embedded_trigger(2);
  hci_driver.state = HCI_STATE_STOP;
  usbhEPCloseS(&hci_event_in_ep);
  usbhEPCloseS(&hci_acl_in_ep);
  usbhEPCloseS(&hci_acl_out_ep);
  osalOsRescheduleS();
  osalSysUnlock();

  return 0;
}


static int usb_send_cmd_packet(uint8_t *packet, int size){

  if (hci_driver.state != HCI_STATE_READY) return -1;

  // prepare transfer
  req.wLength = size;
  hci_ctrl_urb.buff = packet;
  hci_ctrl_urb.requestedLength = size;

  // update status before submitting transfer
  usb_command_active = 1;

  // submit transfer
  osalSysLock();
  usbhURBObjectResetI(&hci_ctrl_urb);
  usbhURBSubmitI(&hci_ctrl_urb);
  osalSysUnlock();

  return 0;
}

static int usb_send_acl_packet(uint8_t *packet, int size){
  if (hci_driver.state != HCI_STATE_READY) return -1;

  // prepare transfer
  hci_acl_out_urb.buff = packet;
  hci_acl_out_urb.requestedLength = size;

  // update status before submitting transfer
  usb_acl_out_active = 1;

  // submit transfer
  osalSysLock();
  usbhURBObjectResetI(&hci_acl_out_urb);
  usbhURBSubmitI(&hci_acl_out_urb);
  osalSysUnlock();

  return 0;
}

static int usb_can_send_packet_now(uint8_t packet_type) {
  switch (packet_type) {
  case HCI_COMMAND_DATA_PACKET:
    return !usb_command_active;
  case HCI_ACL_DATA_PACKET:
    return !usb_acl_out_active;
  default:
    return 0;
  }
}

int usb_send_packet(uint8_t packet_type, uint8_t * packet, int size) {
  switch (packet_type) {
  case HCI_COMMAND_DATA_PACKET:
    return usb_send_cmd_packet(packet, size);
  case HCI_ACL_DATA_PACKET:
    return usb_send_acl_packet(packet, size);
  default:
    return -1;
  }
}

static void usb_register_packet_handler(void (*handler)(uint8_t packet_type, uint8_t *packet, uint16_t size)){
  uinfo("registering packet handler");
  packet_handler = handler;
}

static void dummy_handler(uint8_t packet_type, uint8_t *packet, uint16_t size){
  (void)packet_type;
  (void)packet;
  (void)size;
}

// get usb singleton
const hci_transport_t * hci_transport_usb_instance(void) {
  return &hci_transport_usb;
}

void hci_transport_usb_init(void){
  memset(&hci_driver, 0, sizeof(hci_driver_t));
  hci_driver.info = &usbhbtdClassDriverInfo;

  memset(&hci_transport_usb, 0, sizeof(hci_transport_t));
  hci_transport_usb.name                          = "H2 USB HCI";
  hci_transport_usb.open                          = usb_open;
  hci_transport_usb.close                         = usb_close;
  hci_transport_usb.register_packet_handler       = usb_register_packet_handler;
  hci_transport_usb.can_send_packet_now           = usb_can_send_packet_now;
  hci_transport_usb.send_packet                   = usb_send_packet;
}

hci_state_e hci_transport_usb_get_state(void){
  return hci_driver.state;
}

#endif
