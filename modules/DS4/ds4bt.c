
#include "ch.h"
#include "hal.h"

#include "btstack_run_loop_embedded.h"
#include "btstack_run_loop.h"
#include "btstack_event.h"
#include "hci_cmd.h"
#include "btstack_memory.h"
#include "hci.h"
#include "l2cap.h"

#include "usbh/internal.h"

#include "ds4.h"
#include "ds4bt.h"

#if USBHBTD_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
//#define udbgf(f, ...)  do {} while(0)
//#define udbg(f, ...)   do {} while(0)
#endif

#if USBHBTD_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
//#define uinfof(f, ...)  do {} while(0)
//#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHBTD_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
//#define uwarnf(f, ...)  do {} while(0)
//#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHBTD_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
//#define uerrf(f, ...)  do {} while(0)
//#define uerr(f, ...)   do {} while(0)
#endif

#define INQUIRY_INTERVAL 5

static bd_addr_t ds4_bt_addr;
static uint16_t l2cap_ctrl_cid = 0;
static uint16_t l2cap_int_cid = 0;
static bool request_full_report = false;
static bool ds4bt_need_write = false;
static bool ds4bt_connected = false;
static bool ds4bt_incoming = false;

static btstack_packet_callback_registration_t hci_event_callback_registration;

static uint8_t buf[79] = {0};
static void ds4bt_send(void){
  buf[0] = 0x52;
  buf[1] = 0x11;
  buf[2] = 0x80;
  buf[4] = 0xFF;
  memcpy(&buf[7], (uint8_t*)&DS4_cmd, sizeof(DS4_command_t));
  l2cap_send(l2cap_ctrl_cid, buf, 79);
  ds4bt_need_write = false;
}

void ds4bt_process_packet(uint8_t *packet, uint16_t size){
  if ((size == 79) && (packet[0] == 0xA1) && (packet[1] == 0x11))
    memcpy(&DS4, &(packet[4]), sizeof(DS4_status_t));
}

static void l2cap_packet_handler(uint8_t packet_type, uint16_t channel,
    uint8_t *packet, uint16_t size) {
  if (packet_type == HCI_EVENT_PACKET) {
    if (hci_event_packet_get_type(packet) == L2CAP_EVENT_CHANNEL_OPENED) {
      if (packet[2]) {
       // uinfo("Connection failed\n\r");
        return;
      } else if (packet[2] == 0) {
        uint16_t psm = little_endian_read_16(packet, 11);
        if (psm == PSM_HID_CONTROL) {
          l2cap_create_channel(l2cap_packet_handler, ds4_bt_addr,
          PSM_HID_INTERRUPT, l2cap_max_mtu(), &l2cap_int_cid);
         // uinfo("Connected\n\r");
          ds4bt_connected = true;
        } else if (psm == PSM_HID_INTERRUPT){
          l2cap_request_can_send_now_event(l2cap_ctrl_cid);
          request_full_report = true;
        }
      }
    } else if (hci_event_packet_get_type(packet) == L2CAP_EVENT_CAN_SEND_NOW
        && l2cap_ctrl_cid == channel) {
      if (request_full_report) {
        uint8_t buf[2];
        buf[0] = 0x43;
        buf[1] = 0x02;
        l2cap_send(l2cap_ctrl_cid, buf, 2);
        request_full_report = false;
        ds4bt_need_write = true;
      }
    }
  } else if (packet_type == L2CAP_DATA_PACKET) {
    ds4bt_process_packet(packet, size);
    if(ds4bt_need_write){
      ds4bt_send();
    }

  }
}

static void packet_handler(uint8_t packet_type, uint16_t channel,
    uint8_t *packet, uint16_t size) {
  (void) channel;
  (void) size;
  switch (packet_type) {
  case HCI_EVENT_PACKET:
    switch (hci_event_packet_get_type(packet)) {

    case BTSTACK_EVENT_STATE:
      // bt stack activated, get started - set local name
      if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
        //uinfo("Inquiry");
        hci_send_cmd(&hci_inquiry, HCI_INQUIRY_LAP, INQUIRY_INTERVAL, 0);
        break;
      }
      break;

    case HCI_EVENT_INQUIRY_RESULT:

      // ignore further results
      if (ds4bt_connected) break;

      // check device class
      if ((packet[12] != 0x08) || (packet[13] != 0x25) || (packet[14] != 0))
        break;

      // flip addr
      reverse_bd_addr(&packet[3], ds4_bt_addr);

      // show addr
      //uinfof("BTADDR: %s", bd_addr_to_str(ds4_bt_addr));

      ds4bt_incoming = true;

      hci_send_cmd(&hci_inquiry_cancel);
      break;

    case HCI_EVENT_INQUIRY_COMPLETE:
      if(!ds4bt_connected){
        //uinfo("inquiry complete, start page scan");
        gap_connectable_control(1);
      }
      break;

    case HCI_EVENT_LINK_KEY_REQUEST:
      // deny link key request
      hci_send_cmd(&hci_link_key_request_negative_reply, &ds4_bt_addr);
      break;

    case HCI_EVENT_PIN_CODE_REQUEST:
      // inform about pin code request
      //uinfo("Enter 0000");
      hci_send_cmd(&hci_pin_code_request_reply, &ds4_bt_addr, 4, "0000");
      break;

    case HCI_EVENT_CONNECTION_COMPLETE:
      if(!ds4bt_incoming){
        reverse_bd_addr(&packet[5], ds4_bt_addr);
        hci_send_cmd(&hci_delete_stored_link_key, ds4_bt_addr, 1);
      }
      ds4bt_incoming = false;
      break;

    case HCI_EVENT_COMMAND_COMPLETE:
      if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_inquiry_cancel)) {
        hci_send_cmd(&hci_write_authentication_enable, 0);
        break;
      }
      if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_write_authentication_enable)) {
        // connect to device
        //uinfo("Connecting to DS4");
        l2cap_create_channel(l2cap_packet_handler, ds4_bt_addr, PSM_HID_CONTROL,
            l2cap_max_mtu(), &l2cap_ctrl_cid);
      }
      if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_delete_stored_link_key)) {
        // connect to device
        //uinfo("Connecting to DS4");
        l2cap_create_channel(l2cap_packet_handler, ds4_bt_addr, PSM_HID_CONTROL,
            l2cap_max_mtu(), &l2cap_ctrl_cid);
      }
      break;

    case HCI_EVENT_DISCONNECTION_COMPLETE:
      ds4bt_connected = false;
      DS4_ResetStatus();
      gap_connectable_control(1);
      break;

    case HCI_EVENT_COMMAND_STATUS:
    case HCI_EVENT_TRANSPORT_PACKET_SENT:
      break;

    default:
      //udbgf("HCI EVT:%02x", hci_event_packet_get_type(packet));
      break;
    }
    break;
  default:
    //uinfof("unhandled packet type:%02x", packet_type);
    break;
  }
}

static THD_WORKING_AREA(waBTD, 8192);
static THD_FUNCTION(BTDThd, arg) {

  (void) arg;
  chRegSetThreadName("BTD Thread");

  btstack_run_loop_init(btstack_run_loop_embedded_get_instance());
  btstack_run_loop_embedded_set_thread_pt(chThdGetSelfX());

  while (TRUE) {
    /// GET STARTED with BTstack ///
    btstack_memory_init();

    // init HCI
    hci_transport_usb_init();

    hci_init(hci_transport_usb_instance(), NULL);

    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // init L2CAP
    l2cap_init();

    while (hci_transport_usb_get_state() != HCI_STATE_ACTIVE) {
      chThdSleepMilliseconds(100);
    }

    //uinfo("Client HCI init done\r\n");

    gap_connectable_control(1);
    gap_ssp_set_authentication_requirement(
        SSP_IO_AUTHREQ_MITM_PROTECTION_REQUIRED_GENERAL_BONDING);
    gap_set_local_name("DS4 Dongle");
    hci_power_control(HCI_POWER_OFF);
    hci_power_control(HCI_POWER_ON);

    // go
    btstack_run_loop_execute();

    //chThdSleepMilliseconds(10);
  }
}

void DS4BT_Start(void){
  chThdCreateStatic(waBTD, sizeof(waBTD), HIGHPRIO-5, BTDThd, NULL);
}

void DS4BT_Notify_Write(void){
  if(ds4bt_connected)
    ds4bt_need_write = true;
}

bool DS4BT_IsConnected(void){
  return ds4bt_connected;
}
