#include "ch.h"
#include "hal.h"
#include "usbh/dev/ds4.h"
#include "ds4.h"
#include "ds4bt.h"
//#include "log.h"
#include <string.h>
#include "app.h"

#define DS4_DEFAULT { \
    .hat_left_x = 127, \
    .hat_left_y = 127, \
    .hat_right_x = 127, \
    .hat_right_y = 127, \
    .dpad_code = DPAD_OFF, \
    .tpad_info[0].finger[0] = {.not_touching = 1}, \
    .tpad_info[0].finger[1] = {.not_touching = 1}, \
    .tpad_info[1].finger[0] = {.not_touching = 1}, \
    .tpad_info[1].finger[1] = {.not_touching = 1}, \
    .tpad_info[2].finger[0] = {.not_touching = 1}, \
    .tpad_info[2].finger[1] = {.not_touching = 1} \
}

DS4_status_t DS4 = DS4_DEFAULT;
DS4_status_t old_data = DS4_DEFAULT;
DS4_command_t DS4_cmd = {.led_r = 255, .led_g = 20, .led_b = 147};
static const DS4_status_t DS4_default = DS4_DEFAULT;

static bool ds4_need_send = false;
static bool ds4_connected = false;


//uint8_t DS4_ButtonPress(ButtonEnum b){
//     if (b <= LEFT){
//         switch (b) {
//             case UP:
//                 return (DS4.dpad_code == DPAD_LEFT_UP && old_data.dpad_code == DPAD_OFF)
//                     || (DS4.dpad_code == DPAD_UP && old_data.dpad_code == DPAD_OFF)
//                     || (DS4.dpad_code == DPAD_UP_RIGHT && old_data.dpad_code == DPAD_OFF);
//             case RIGHT:
//                 return (DS4.dpad_code == DPAD_UP_RIGHT && old_data.dpad_code == DPAD_OFF)
//                     || (DS4.dpad_code == DPAD_RIGHT && old_data.dpad_code == DPAD_OFF)
//                     || (DS4.dpad_code == DPAD_RIGHT_DOWN && old_data.dpad_code == DPAD_OFF);
//             case DOWN:
//                 return (DS4.dpad_code == DPAD_RIGHT_DOWN && old_data.dpad_code == DPAD_OFF)
//                     || (DS4.dpad_code == DPAD_DOWN && old_data.dpad_code == DPAD_OFF)
//                     || (DS4.dpad_code == DPAD_DOWN_LEFT && old_data.dpad_code == DPAD_OFF);
//             case LEFT:
//                 return (DS4.dpad_code == DPAD_DOWN_LEFT && old_data.dpad_code == DPAD_OFF)
//                     || (DS4.dpad_code == DPAD_LEFT && old_data.dpad_code == DPAD_OFF)
//                     || (DS4.dpad_code == DPAD_LEFT_UP && old_data.dpad_code == DPAD_OFF);
//             default:
//                 return 0;
//         }
//     }else{
//       switch (b){
//             case SHARE:
//               return DS4.share & !(old_data.share);
//             case OPTIONS:
//               return DS4.options & !(old_data.options);
//             case L3:
//               return DS4.l3 & !(old_data.l3);
//             case R3:
//               return DS4.r3 & !(old_data.r3);
//             case L2:
//               return DS4.l2 & !(old_data.l2);
//             case R2:
//               return DS4.r2 & !(old_data.r2);
//             case L1:
//               return DS4.l1 & !(old_data.l1);
//             case R1:
//               return DS4.r1 & !(old_data.r1);
//             case TRIANGLE:
//               return DS4.triangle & !(old_data.triangle);
//             case CROSS:
//               return DS4.cross & !(old_data.cross);
//             case SQUARE:
//               return DS4.square & !(old_data.square);
//             case PS:
//               return DS4.ps & !(old_data.ps);
//             case TOUCHPAD:
//                return DS4.tpad_click & !(old_data.tpad_click);
//             case CIRCLE:
//                return DS4.circle & !(old_data.circle);
//             default:
//                return 0;
//       }
//     }
//}

static void ds4_process_packet(uint8_t *packet, uint16_t size){
  if ((size == 64) && (packet[0] == 0x01))
    memcpy(&DS4, &(packet[1]), sizeof(DS4_status_t));
  else if ((size == 79) && (packet[0] == 0x11))
    memcpy(&DS4, &(packet[3]), sizeof(DS4_status_t));
//  ui_event evt;
//  bool need_post = false;
//
//  if (DS4.cross ){
//          evt.type = UI_INPUT_BUTTON;
//          evt.data.button_state = UI_BUTTON_DOWN;
//          need_post = true;
//  }
////  }else if (DS4_ButtonPress(UP)){
////         evt.type = UI_INPUT_BUTTON;
////         evt.data.button_state = UI_BUTTON_UP;
////         need_post = true;
////  }else if(DS4_ButtonPress(CIRCLE)){
////         evt.type = UI_INPUT_BUTTON;
////         evt.data.button_state = UI_BUTTON_ENTER;
////         need_post = true;
////  }else if(DS4_ButtonPress(LEFT)){
////         evt.type = UI_INPUT_BUTTON;
////         evt.data.button_state = UI_BUTTON_BACK;
////         need_post = true;
////  }
////
//  if(need_post){
//     chMBPost(&app_mb, (msg_t)&evt, TIME_IMMEDIATE);
//     need_post = false;
//   }
//
  //memcpy(&old_data, &DS4, sizeof(old_data));
}

static THD_WORKING_AREA(waDS4, 1024);
static THD_FUNCTION(DS4Thd, arg) {
  (void)arg;
  chRegSetThreadName("DS4 Thread");

  USBHDS4Driver *const ds4p = &USBHDS4[0];

  ds4p->packet_handler = ds4_process_packet;
  DS4_ResetStatus();

  while (TRUE) {
    switch (ds4p->state) {
    case USBHDS4_STATE_UNINIT:
    case USBHDS4_STATE_STOP:
      if(ds4_connected){
        //log_i("\nDS4 Disconnected or Stopped\n");
        DS4_ResetStatus();
      }
      ds4_connected = false;
      chThdSleepMilliseconds(500);
      break;
    case USBHDS4_STATE_ACTIVE:
      //log_i("\nDS4 connected\n");
      ds4_connected = true;
      usbhds4Start(ds4p);
      chThdSleepMilliseconds(20);
      usbhds4Send(ds4p, (uint8_t*)&DS4_cmd);
      chThdSleepMilliseconds(100);
      break;
    case USBHDS4_STATE_READY:
      if(ds4_need_send){
        usbhds4Send(ds4p, (uint8_t*)&DS4_cmd);
        ds4_need_send = false;
      }
      break;
    }
    chThdSleepMilliseconds(60);
  }
}

void DS4_Start(void){
  usbhds4ObjectInit(&USBHDS4[0]);
  chThdCreateStatic(waDS4, sizeof(waDS4), NORMALPRIO+1,
                        DS4Thd, NULL);
}

void DS4_SetLED(uint8_t r, uint8_t g, uint8_t b){
  DS4_cmd.led_r = r;
  DS4_cmd.led_g = g;
  DS4_cmd.led_b = b;
  ds4_need_send = true;
  DS4BT_Notify_Write();
}

void DS4_SetFlash(uint8_t ontime, uint8_t offtime){
  DS4_cmd.flash_on = ontime;
  DS4_cmd.flash_off = offtime;
  ds4_need_send = true;
  DS4BT_Notify_Write();
}

void DS4_SetRumble(uint8_t small, uint8_t big){
  DS4_cmd.rumble_small = small;
  DS4_cmd.rumble_big = big;
  ds4_need_send = true;
  DS4BT_Notify_Write();
}

bool DS4_IsConnected(void){
  return (ds4_connected | DS4BT_IsConnected());
}

void DS4_ResetStatus(void){
  memcpy(&DS4, &DS4_default, sizeof(DS4_status_t));
}
