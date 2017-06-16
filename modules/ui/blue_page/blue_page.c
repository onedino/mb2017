//<<<<<<< HEAD
////<<<<<<< HEAD
/////*
//// * start_robot_main_page.c
//// *
//// *  Created on: 21 Dec 2015
//// *      Author: u564
//// */
////
////#include "ch.h"
////#include "hal.h"
////#include "gfx.h"
////#include "app.h"
////#include "auto_path.h"
////#include "pid.h"
////
////#include "gui.h"
////
////#include "stdio.h"
////#include "usage.h"
////#if  USE_MOTOR_0  ||  USE_MOTOR_1  ||  USE_MOTOR_2  ||  USE_MOTOR_3  ||  USE_MOTOR_4  ||  USE_MOTOR_5  ||  USE_MOTOR_6  ||  USE_MOTOR_7
////    #include "motor.h"
////#endif
////#if USE_ENCODER
////#include "encoder.h"
////#endif
////#if USE_SERVO
////#include "servo.h"
////#endif
////#if USE_LINESENSOR_0 || USE_LINESENSOR_1 || USE_LINESENSOR_2 || USE_LINESENSOR_3
////#include "linesensor.h"
////#endif
////#include "loop_stats.h"
////#include "custom_draw.h"
////#include "widgets.h"
////#include "status_bar.h"
////char * UIDrivingState[2] = {"activated","deactivated"}; //for ui
////
////// GHandles
////GHandle ContainerPage0;
////GHandle Console;
////GHandle Console1;
////GHandle Console2;
////
////// Fonts
////font_t dejavu_sans_16_anti_aliased;
////font_t dejavu_sans_10;
////font_t dejavu_sans_12_anti_aliased;
////font_t dejavu_sans_24_anti_aliased;
////font_t dejavu_sans_32_anti_aliased;
////
////static void createPagePage0(void)
////{
////  // create container widget: ghContainerPage0
////  ContainerPage0 = createContainer(0, STATUS_BAR_HEIGHT, GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT - STATUS_BAR_HEIGHT, FALSE);
////
////  // Create console widget: ghConsole
////  Console = createConsole(&ContainerPage0, 0, 0, 270, 130);
////  gwinSetColor(Console, Silver);
////  gwinSetBgColor(Console, Black);
////  //gwinSetFont(ghConsole, dejavu_sans_32_anti_aliased);
////  gwinRedraw(Console);
////
////  Console2 = createConsole(&ContainerPage0, 0, 131, 270, 260);
////  gwinRedraw(Console2);
////
////  Console1 = createConsole(&ContainerPage0, 270,0,270,130);
////  gwinSetColor(Console1, Silver);
////  gwinSetBgColor(Console1, Black);
////  gwinSetFont(Console1, dejavu_sans_32_anti_aliased);
////  gwinRedraw(Console1);
////}
////
////void guiShowPage(unsigned pageIndex)
////{
////  // Hide all pages
////  gwinHide(ContainerPage0);
////
////  // Show page selected page
////  switch (pageIndex) {
////  case 0:
////    gwinShow(ContainerPage0);
////    break;
////
////  default:
////    break;
////  }
////}
////
////void guiCreate(void)
////{
////  //GWidgetInit wi;
////
////  // Prepare fonts
////  dejavu_sans_16_anti_aliased = gdispOpenFont("DejaVuSans16_aa");
////  dejavu_sans_10 = gdispOpenFont("DejaVuSans10");
////  dejavu_sans_12_anti_aliased = gdispOpenFont("DejaVuSans12_aa");
////  dejavu_sans_24_anti_aliased = gdispOpenFont("DejaVuSans24_aa");
////  dejavu_sans_32_anti_aliased = gdispOpenFont("DejaVuSans32_aa");
////
////  // Prepare images
////
////  // GWIN settings
//////  gwinSetDefaultFont(dejavu_sans_32_anti_aliased);
//////  gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
//////  gwinSetDefaultColor(Black);
//////  gwinSetDefaultBgColor(White);
////
////  // Create all the display pages
////  createPagePage0();
////
////  // Select the default display page
////  guiShowPage(0);
////
////  // Console sample text
////  //gwinPrintf(ghConsole, "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet");
////}
////
////void guiUpdate(){
////     gwinClear(Console1);
////     gwinPrintf(Console1, UIDrivingState[DrivingState]);
////     gwinClear(Console);
//////     gwinPrintf(Console, "Setpoint0: %d\r\n", M[0].SetPoint);
//////     gwinPrintf(Console, "Setpoint1: %d\r\n", M[1].SetPoint);
//////     gwinPrintf(Console, "Setpoint2: %d\r\n", M[2].SetPoint);
//////     gwinPrintf(Console, "Setpoint3: %d\r\n", M[3].SetPoint);
////     gwinClear(Console2);
////     gwinPrintf(Console2, "x: %d\r\n", x);
////     gwinPrintf(Console2, "y: %d\r\n", y);
////     gwinPrintf(Console2, "a: %d\r\n", tRotation);
//////     gwinPrintf(Console2, "fb0: %d\r\n",M[0].Feedback);
//////     gwinPrintf(Console2, "fb1: %d\r\n",M[1].Feedback);
////     gwinPrintf(Console2, "E0: %d\r\n",E0);
////     gwinPrintf(Console2, "E1: %d\r\n",E1);
////     gwinPrintf(Console2, "E2: %d\r\n",E2);
////
////
////}
////
//////static char buf[8];
//////
//////#define VAL_UPDATE(handle, ...) \
//////  snprintf(buf, 8, __VA_ARGS__); \
//////  gwinSetText(handle, buf, TRUE);
//////
//////static void guiUpdate(void){
//////  //refresh encoder values
//////  VAL_UPDATE(ghLabelE1_count, "%+05d", encoder1_2.delta_count[0]);
//////  VAL_UPDATE(ghLabelE2_count, "%+05d", encoder1_2.delta_count[1]);
//////  VAL_UPDATE(ghLabelE3_count, "%+05d", encoder3_4.delta_count[0]);
//////  VAL_UPDATE(ghLabelE4_count, "%+05d", encoder3_4.delta_count[1]);
//////
//////  //refresh motor values
//////  VAL_UPDATE(ghLabelM0_Setpoint, "%+05d", M[0].SetPoint);
//////  VAL_UPDATE(ghLabelM0_Feedback, "%+05d", M[0].Feedback);
//////  VAL_UPDATE(ghLabelM0_Current, "%05d", M[0].Board.Current);
//////
//////  VAL_UPDATE(ghLabelM1_Setpoint, "%+05d", M[1].SetPoint);
//////  VAL_UPDATE(ghLabelM1_Feedback, "%+05d", M[1].Feedback);
//////  VAL_UPDATE(ghLabelM1_Current, "%05d", M[1].Board.Current);
//////
//////  VAL_UPDATE(ghLabelM2_Setpoint, "%+05d", M[2].SetPoint);
//////  VAL_UPDATE(ghLabelM2_Feedback, "%+05d", M[2].Feedback);
//////  VAL_UPDATE(ghLabelM2_Current, "%05d", M[2].Board.Current);
//////
//////  VAL_UPDATE(ghLabelM3_Setpoint, "%+05d", M[3].SetPoint);
//////  VAL_UPDATE(ghLabelM3_Feedback, "%+05d", M[3].Feedback);
//////  VAL_UPDATE(ghLabelM3_Current, "%05d", M[3].Board.Current);
//////
//////  VAL_UPDATE(ghLabelM4_Setpoint, "%+05d", M[4].SetPoint);
//////  VAL_UPDATE(ghLabelM4_Feedback, "%+05d", M[4].Feedback);
//////  VAL_UPDATE(ghLabelM4_Current, "%05d", M[4].Board.Current);
//////
//////  VAL_UPDATE(ghLabelM5_Setpoint, "%+05d", M[5].SetPoint);
//////  VAL_UPDATE(ghLabelM5_Feedback, "%+05d", M[5].Feedback);
//////  VAL_UPDATE(ghLabelM5_Current, "%05d", M[5].Board.Current);
//////
//////  VAL_UPDATE(ghLabelM6_Setpoint, "%+05d", M[6].SetPoint);
//////  VAL_UPDATE(ghLabelM6_Feedback, "%+05d", M[6].Feedback);
//////  VAL_UPDATE(ghLabelM6_Current, "%05d", M[6].Board.Current);
//////
//////  VAL_UPDATE(ghLabelM7_Setpoint, "%+05d", M[7].SetPoint);
//////  VAL_UPDATE(ghLabelM7_Feedback, "%+05d", M[7].Feedback);
//////  VAL_UPDATE(ghLabelM7_Current, "%05d", M[7].Board.Current);
//////
//////
//////  VAL_UPDATE(ghLabellp_avg_val, "%05d", (int)loop_stats.avg_loop_us);
//////  VAL_UPDATE(ghLabellp_max_val, "%05d", (int)loop_stats.max_loop_us);
//////  VAL_UPDATE(ghLabelcomm_avg_val, "%05d", (int)loop_stats.avg_comm_us);
//////  VAL_UPDATE(ghLabelcomm_max_val, "%05d", (int)loop_stats.max_comm_us);
//////
//////
//////  VAL_UPDATE(ghLabelCPU_VAL, "%02d", (int) cpu_usage_get_recent());
//////}
////
////void start_robot_main(void *params){
////  (void) params;
////
////  ui_event *evt = NULL;
////
////  guiCreate();
////  //ActivateDriving();//lock motor
////  while(TRUE){
////
////    if(chMBFetch(&app_mb, (msg_t*)(&evt), TIME_IMMEDIATE) == MSG_OK){
////
////      switch(evt->type){
////        case UI_INPUT_BUTTON:
////          if(evt->data.button_state == UI_BUTTON_BACK){
////            return;
////          } else if(evt->data.button_state == UI_BUTTON_ENTER){
////          } else {
////          }
////          break;
////        case UI_STATUSBAR_TICK:
////             status_bar_redraw();
////          break;
////        default:
////          break;
////      }
////    }
////    guiUpdate();
////    chThdSleepMilliseconds(60);
////  }
////
////}
////
////
////application start_robot = {
////    .name = "Start robot",
////    .main = start_robot_main
////};
////=======
///*
// * start_robot_main_page.c
// *
// *  Created on: 21 Dec 2015
// *      Author: u564
// */
//
//#include "ch.h"
//#include "hal.h"
//#include "gfx.h"
//#include "app.h"
//
//#include "start_robot_page_gui.h"
//
//#include "stdio.h"
//#include "usage.h"
//#if  USE_MOTOR_0  ||  USE_MOTOR_1  ||  USE_MOTOR_2  ||  USE_MOTOR_3  ||  USE_MOTOR_4  ||  USE_MOTOR_5  ||  USE_MOTOR_6  ||  USE_MOTOR_7
//    #include "motor.h"
//#endif
//#if USE_ENCODER
//#include "encoder.h"
//#endif
//#if USE_SERVO
//#include "servo.h"
//#endif
//#if USE_LINESENSOR_0 || USE_LINESENSOR_1 || USE_LINESENSOR_2 || USE_LINESENSOR_3
//#include "linesensor.h"
//#endif
//#include "loop_stats.h"
//#include "custom_draw.h"
//#include "log.h"
//#include "ds4.h"
//#include "status_bar.h"
//
//
//static char buf[8];
//
//#define VAL_UPDATE(handle, ...) do {           \
//  snprintf(buf, 8, __VA_ARGS__);               \
//  gwinSetText(handle, buf, TRUE);              \
//} while(0)
//
//static void guiUpdate(void){
//  //refresh encoder values
//  VAL_UPDATE(ghLabelE1_count, "%+05d", encoder1_2.delta_count[0]);
//  VAL_UPDATE(ghLabelE2_count, "%+05d", encoder1_2.delta_count[1]);
//  VAL_UPDATE(ghLabelE3_count, "%+05d", encoder3_4.delta_count[0]);
//  VAL_UPDATE(ghLabelE4_count, "%+05d", encoder3_4.delta_count[1]);
//
//  //refresh motor values
//  VAL_UPDATE(ghLabelM0_Setpoint, "%+05d", M[0].SetPoint);
//  if(M[0].timeout)
//    VAL_UPDATE(ghLabelM0_Feedback, "%+05d", M[0].Feedback);
//  else
//    VAL_UPDATE(ghLabelM0_Feedback, "TIMEOUT");
//  VAL_UPDATE(ghLabelM0_Current, "%05d", M[0].Board.Current);
//
//  VAL_UPDATE(ghLabelM1_Setpoint, "%+05d", M[1].SetPoint);
//  if(M[1].timeout)
//    VAL_UPDATE(ghLabelM1_Feedback, "%+05d", M[1].Feedback);
//  else
//    VAL_UPDATE(ghLabelM1_Feedback, "TIMEOUT");
//  VAL_UPDATE(ghLabelM1_Current, "%05d", M[1].Board.Current);
//
//  VAL_UPDATE(ghLabelM2_Setpoint, "%+05d", M[2].SetPoint);
//  if(M[2].timeout)
//    VAL_UPDATE(ghLabelM2_Feedback, "%+05d", M[2].Feedback);
//  else
//    VAL_UPDATE(ghLabelM2_Feedback, "TIMEOUT");
//  VAL_UPDATE(ghLabelM2_Current, "%05d", M[2].Board.Current);
//
//  VAL_UPDATE(ghLabelM3_Setpoint, "%+05d", M[3].SetPoint);
//  if(M[3].timeout)
//    VAL_UPDATE(ghLabelM3_Feedback, "%+05d", M[3].Feedback);
//  else
//    VAL_UPDATE(ghLabelM3_Feedback, "TIMEOUT");
//  VAL_UPDATE(ghLabelM3_Current, "%05d", M[3].Board.Current);
//
//  VAL_UPDATE(ghLabelM4_Setpoint, "%+05d", M[4].SetPoint);
//  if(M[4].timeout)
//    VAL_UPDATE(ghLabelM4_Feedback, "%+05d", M[4].Feedback);
//  else
//    VAL_UPDATE(ghLabelM4_Feedback, "TIMEOUT");
//  VAL_UPDATE(ghLabelM4_Current, "%05d", M[4].Board.Current);
//
//  VAL_UPDATE(ghLabelM5_Setpoint, "%+05d", M[5].SetPoint);
//  if(M[5].timeout)
//    VAL_UPDATE(ghLabelM5_Feedback, "%+05d", M[5].Feedback);
//  else
//    VAL_UPDATE(ghLabelM5_Feedback, "TIMEOUT");
//  VAL_UPDATE(ghLabelM5_Current, "%05d", M[5].Board.Current);
//
//  VAL_UPDATE(ghLabelM6_Setpoint, "%+05d", M[6].SetPoint);
//  if(M[6].timeout)
//    VAL_UPDATE(ghLabelM6_Feedback, "%+05d", M[6].Feedback);
//  else
//    VAL_UPDATE(ghLabelM6_Feedback, "TIMEOUT");
//  VAL_UPDATE(ghLabelM6_Current, "%05d", M[6].Board.Current);
//
//  VAL_UPDATE(ghLabelM7_Setpoint, "%+05d", M[7].SetPoint);
//  if(M[7].timeout)
//    VAL_UPDATE(ghLabelM7_Feedback, "%+05d", M[7].Feedback);
//  else
//    VAL_UPDATE(ghLabelM7_Feedback, "TIMEOUT");
//  VAL_UPDATE(ghLabelM7_Current, "%05d", M[7].Board.Current);
//
//
//  VAL_UPDATE(ghLabellp_avg_val, "%05d", (int)loop_stats.avg_loop_us);
//  VAL_UPDATE(ghLabellp_max_val, "%05d", (int)loop_stats.max_loop_us);
//  VAL_UPDATE(ghLabelcomm_avg_val, "%05d", (int)loop_stats.avg_comm_us);
//  VAL_UPDATE(ghLabelcomm_max_val, "%05d", (int)loop_stats.max_comm_us);
//
//
//  VAL_UPDATE(ghLabelCPU_VAL, "%02d", (int) cpu_usage_get_recent());
//
//  gwinCheckboxCheck(ghCheckboxDS4, DS4_IsConnected());
//}
//
//void start_robot_main(void *params){
//  (void) params;
//
//  ui_event *evt = NULL;
//
//  guiCreate();
//
//  while(TRUE){
//
//    if(chMBFetch(&app_mb, (msg_t*)(&evt), TIME_IMMEDIATE) == MSG_OK){
//
//      switch(evt->type){
//        //case UI_DS4_BUTTON:
//        case UI_INPUT_BUTTON:
//          if(evt->data.button_state == UI_BUTTON_BACK){
//            //gwinDestroy(ghConsole);
//            //ghConsole = NULL;
//            return;
//          } else if(evt->data.button_state == UI_BUTTON_ENTER){
//            //log_i("LoLLoLLoLLoLLoLLoLLoLLoL\n");
//          } else {
//          }
//          break;
//        case UI_STATUSBAR_TICK:
//           status_bar_redraw();
//          break;
//        default:
//
//          break;
//      }
//    }
//    guiUpdate();
//    chThdSleepMilliseconds(60);
//  }
//
//}
//
//
//application start_robot = {
//    .name = "Start robot",
//    .main = start_robot_main
//};
//
//=======
/*
 * start_robot_main_page.c
 *
 *  Created on: 21 Dec 2015
 *      Author: u564
 */



#include <blue.h>
#include "ch.h"
#include "hal.h"
#include "gfx.h"
#include "app.h"
#include "pid.h"
#include "analog.h"
//#include "gui.h"
//#include "ps4_usbhost.h"
#include "ds4.h"
#include "stdio.h"
#include "usage.h"
#if  USE_MOTOR_0  ||  USE_MOTOR_1  ||  USE_MOTOR_2  ||  USE_MOTOR_3  ||  USE_MOTOR_4  ||  USE_MOTOR_5  ||  USE_MOTOR_6  ||  USE_MOTOR_7
    #include "motor.h"
#endif
#if USE_ENCODER
#include "encoder.h"
#endif
#if USE_SERVO
#include "servo.h"
#endif
#if USE_LINESENSOR_0 || USE_LINESENSOR_1 || USE_LINESENSOR_2 || USE_LINESENSOR_3
#include "linesensor.h"
#endif
#if USE_AIRBOARD_0
#include "airboard.h"
#endif
#include "loop_stats.h"
#include "custom_draw.h"
#include "widgets.h"
#include "status_bar.h"
#include "common.h"
char * UIDrivingState[2] = {"activated","deactivated"}; //for ui

// GHandles
GHandle ContainerPage0;
GHandle Console;
GHandle Console1;
GHandle Console2;
GHandle Console3;
GHandle Console4;
GHandle Console5;

//// Fonts
//font_t dejavu_sans_16_anti_aliased;
//font_t dejavu_sans_10;
//font_t dejavu_sans_12_anti_aliased;
//font_t dejavu_sans_24_anti_aliased;
//font_t dejavu_sans_32_anti_aliased;

static void createPagePage0_b(void)
{
  // create container widget: ghContainerPage0
  ContainerPage0 = createContainer(0, STATUS_BAR_HEIGHT, GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT - STATUS_BAR_HEIGHT, FALSE);

  // Create console widget: ghConsole
  Console = createConsole(&ContainerPage0, 0, 0, 800, 124);
  gwinSetColor(Console, Silver);
  gwinSetBgColor(Console, Black);
  //gwinSetFont(ghConsole, dejavu_sans_32_anti_aliased);
  gwinRedraw(Console);

  Console2 = createConsole(&ContainerPage0, 0, 125, 200, 350);
  gwinRedraw(Console2);

  Console3 = createConsole(&ContainerPage0, 200, 125, 200, 350);
  gwinRedraw(Console3);

  Console4 = createConsole(&ContainerPage0, 400, 125, 200, 350);
  gwinRedraw(Console4);

  Console5 = createConsole(&ContainerPage0, 600, 125, 200, 350);
  gwinSetColor(Console5, Orange);
  gwinSetBgColor(Console5, Blue);
  gwinRedraw(Console5);

//  Console1 = createConsole(&ContainerPage0, 270,0,270,130);
//  gwinRedraw(Console1);
}

void guiShowPage_b(unsigned pageIndex)
{
  // Hide all pages
  gwinHide(ContainerPage0);

  // Show page selected page
  switch (pageIndex) {
  case 0:
    gwinShow(ContainerPage0);
    break;

  default:
    break;
  }
}

void guiCreate_b(void)
{
  //GWidgetInit wi;

  // Prepare fonts
//  dejavu_sans_16_anti_aliased = gdispOpenFont("DejaVuSans16_aa");
//  dejavu_sans_10 = gdispOpenFont("DejaVuSans10");
//  dejavu_sans_12_anti_aliased = gdispOpenFont("DejaVuSans12_aa");
//  dejavu_sans_24_anti_aliased = gdispOpenFont("DejaVuSans24_aa");
//  dejavu_sans_32_anti_aliased = gdispOpenFont("DejaVuSans32_aa");

  // Prepare images

  // GWIN settings
//  gwinSetDefaultFont(dejavu_sans_32_anti_aliased);
//  gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
//  gwinSetDefaultColor(Black);
//  gwinSetDefaultBgColor(White);

  // Create all the display pages
  createPagePage0_b();

  // Select the default display page
  guiShowPage_b(0);

  // Console sample text
  //gwinPrintf(ghConsole, "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet");
}

void guiUpdate_b(void){
//	RunPath();
	gwinClear(Console);
	gwinPrintf(Console, "SHT  %4d\033t ", M[4].SetPoint);
	gwinPrintf(Console, "PIT  %4d\033t ", getPitch());//M[5].Board.ADCValue
	gwinPrintf(Console, "ROL  %4d\033t ", getRoll());//M[4].Board.ADCValue
	gwinPrintf(Console, "XXX  %4d\033t ", distanceSum);
	gwinPrintf(Console, "YDS  %4d\033t\r\n", (int)yDistance);
//    gwinPrintf(Console, "D_S %4d\033t ", blueStateSet[targetPosition].shootspd - M[4].SetPoint);
//    gwinPrintf(Console, "D_P %4d\033t ", blueStateSet[targetPosition].pitch - getPitch());
//    gwinPrintf(Console, "D_R %4d\033t ", blueStateSet[targetPosition].roll - getRoll());
    gwinPrintf(Console, "D_X %4d\033t\r\n", blueStateSet[targetPosition].x - distanceSum);
    gwinPrintf(Console, "D_Y %4d\033t\r\n", (int)yDistance - blueStateSet[targetPosition].y);
//	gwinPrintf(Console, "SPUS2 %3d\033t  ", Servo1.command[2]);
//	gwinPrintf(Console, "SPIT1 %3d\033t  ", Servo1.command[1]);
//	gwinPrintf(Console, "SROL0 %3d\033t  ", Servo1.command[0]);
//	gwinPrintf(Console, "ALIVE %3d\033t  \r\n", shooterAlive);
//	gwinPrintf(Console, "M01X %4d", (int)(M[0].Feedback));
//	gwinPrintf(Console, ",%4d", (int)(M[1].Feedback));
//	gwinPrintf(Console, " M23U %4d", (int)(M[2].Feedback));
//	gwinPrintf(Console, ",%4d", (int)(M[3].Feedback));
//	gwinPrintf(Console, " M45S %4d", (int)(M[4].Feedback));
//	gwinPrintf(Console, ",%4d", (int)(M[5].Feedback));
//	gwinPrintf(Console, " M6Y %4d\r\n", (int)(M[6].Feedback));

//	gwinClear(Console1);
    gwinClear(Console2);
    gwinPrintf(Console2, "TRGT VAL\r\n");
    gwinPrintf(Console2, "Pos: %d\r\n", blueStateSet[targetPosition].id);
    gwinPrintf(Console2, "Pit: %d\r\n", blueStateSet[targetPosition].pitch);
    gwinPrintf(Console2, "Rol: %d\r\n", blueStateSet[targetPosition].roll);
    gwinPrintf(Console2, "Sht: %d\r\n", blueStateSet[targetPosition].shootspd);
    gwinPrintf(Console2, "Xxx: %d\r\n", blueStateSet[targetPosition].x);
    gwinPrintf(Console2, "Yyy: %d\r\n", blueStateSet[targetPosition].y);


	gwinClear(Console3);
	gwinPrintf(Console3, "Ldis: %d\r\n", Ldisc);
	gwinPrintf(Console3, "Rdis: %d\r\n", Rdisc);
	gwinPrintf(Console3, "Cdis: %d\r\n", Cdisc);
	gwinPrintf(Console3, "A6_R: %d\r\n", A6on);
	gwinPrintf(Console3, "A7_L: %d\r\n", A7on);

	gwinClear(Console4);
	gwinPrintf(Console4, "D0: %d\r\n", debug_display[0]);
	gwinPrintf(Console4, "D1: %d\r\n", debug_display[1]);
	gwinPrintf(Console4, "D2: %d\r\n", debug_display[2]);
	gwinPrintf(Console4, "D3: %d\r\n", debug_display[3]);
	gwinPrintf(Console4, "D4: %d\r\n", debug_display[4]);
	gwinPrintf(Console4, "D5: %d\r\n", debug_display[5]);
	gwinPrintf(Console4, "D6: %d\r\n", debug_display[6]);
	gwinPrintf(Console4, "D7: %d\r\n", debug_display[7]);
	gwinPrintf(Console4, "D8: %d\r\n", debug_display[8]);
	gwinPrintf(Console4, "D9: %d\r", debug_display[9]);

	gwinClear(Console5);
	gwinPrintf(Console5, "BLUE FIELD \r\n");
	if(encoder1_2.Alive <= 0) gwinPrintf(Console5, "EN GG\r\n");
	if(M[0].timeout <= 0) gwinPrintf(Console5, "M0 GG\r\n");
	if(M[1].timeout <= 0) gwinPrintf(Console5, "M1 GG\r\n");
	if(M[2].timeout <= 0) gwinPrintf(Console5, "M2 GG\r\n");
	if(M[3].timeout <= 0) gwinPrintf(Console5, "M3 GG\r\n");
	if(M[4].timeout <= 0) gwinPrintf(Console5, "M4 GG\r\n");
	if(M[5].timeout <= 0) gwinPrintf(Console5, "M5 GG\r\n");
	if(M[6].timeout <= 0) gwinPrintf(Console5, "M6 GG\r\n");
	if(Servo1.Alive <= 0) gwinPrintf(Console5, "S1 GG\r\n");
//	if(airBoard.Alive <= 0) gwinPrintf(Console5, "AIR: %d\r\n");
}

//static char buf[8];
//
//#define VAL_UPDATE(handle, ...)
//  snprintf(buf, 8, __VA_ARGS__);
//  gwinSetText(handle, buf, TRUE);
//
//static void guiUpdate(void){
//  //refresh encoder values
//  VAL_UPDATE(ghLabelE1_count, "%+05d", encoder1_2.delta_count[0]);
//  VAL_UPDATE(ghLabelE2_count, "%+05d", encoder1_2.delta_count[1]);
//  VAL_UPDATE(ghLabelE3_count, "%+05d", encoder3_4.delta_count[0]);
//  VAL_UPDATE(ghLabelE4_count, "%+05d", encoder3_4.delta_count[1]);
//
//  //refresh motor values
//  VAL_UPDATE(ghLabelM0_Setpoint, "%+05d", M[0].SetPoint);
//  VAL_UPDATE(ghLabelM0_Feedback, "%+05d", M[0].Feedback);
//  VAL_UPDATE(ghLabelM0_Current, "%05d", M[0].Board.Current);
//
//  VAL_UPDATE(ghLabelM1_Setpoint, "%+05d", M[1].SetPoint);
//  VAL_UPDATE(ghLabelM1_Feedback, "%+05d", M[1].Feedback);
//  VAL_UPDATE(ghLabelM1_Current, "%05d", M[1].Board.Current);
//
//  VAL_UPDATE(ghLabelM2_Setpoint, "%+05d", M[2].SetPoint);
//  VAL_UPDATE(ghLabelM2_Feedback, "%+05d", M[2].Feedback);
//  VAL_UPDATE(ghLabelM2_Current, "%05d", M[2].Board.Current);
//
//  VAL_UPDATE(ghLabelM3_Setpoint, "%+05d", M[3].SetPoint);
//  VAL_UPDATE(ghLabelM3_Feedback, "%+05d", M[3].Feedback);
//  VAL_UPDATE(ghLabelM3_Current, "%05d", M[3].Board.Current);
//
//  VAL_UPDATE(ghLabelM4_Setpoint, "%+05d", M[4].SetPoint);
//  VAL_UPDATE(ghLabelM4_Feedback, "%+05d", M[4].Feedback);
//  VAL_UPDATE(ghLabelM4_Current, "%05d", M[4].Board.Current);
//
//  VAL_UPDATE(ghLabelM5_Setpoint, "%+05d", M[5].SetPoint);
//  VAL_UPDATE(ghLabelM5_Feedback, "%+05d", M[5].Feedback);
//  VAL_UPDATE(ghLabelM5_Current, "%05d", M[5].Board.Current);
//
//  VAL_UPDATE(ghLabelM6_Setpoint, "%+05d", M[6].SetPoint);
//  VAL_UPDATE(ghLabelM6_Feedback, "%+05d", M[6].Feedback);
//  VAL_UPDATE(ghLabelM6_Current, "%05d", M[6].Board.Current);
//
//  VAL_UPDATE(ghLabelM7_Setpoint, "%+05d", M[7].SetPoint);
//  VAL_UPDATE(ghLabelM7_Feedback, "%+05d", M[7].Feedback);
//  VAL_UPDATE(ghLabelM7_Current, "%05d", M[7].Board.Current);
//
//
//  VAL_UPDATE(ghLabellp_avg_val, "%05d", (int)loop_stats.avg_loop_us);
//  VAL_UPDATE(ghLabellp_max_val, "%05d", (int)loop_stats.max_loop_us);
//  VAL_UPDATE(ghLabelcomm_avg_val, "%05d", (int)loop_stats.avg_comm_us);
//  VAL_UPDATE(ghLabelcomm_max_val, "%05d", (int)loop_stats.max_comm_us);
//
//
//  VAL_UPDATE(ghLabelCPU_VAL, "%02d", (int) cpu_usage_get_recent());
//}

void blue_main(void *params){
  (void) params;

  ui_event *evt = NULL;

  guiCreate_b();
  //ActivateDriving();//lock motor
  while(TRUE){

    if(chMBFetch(&app_mb, (msg_t*)(&evt), TIME_IMMEDIATE) == MSG_OK){

      switch(evt->type){
        case UI_INPUT_BUTTON:
          if(evt->data.button_state == UI_BUTTON_BACK){
            return;
          } else if(evt->data.button_state == UI_BUTTON_ENTER){
          } else {
          }
          break;
        case UI_STATUSBAR_TICK:
             status_bar_redraw();
          break;
        default:
          break;
      }
    }
    guiUpdate_b();
    chThdSleepMilliseconds(60);
  }

}


application blue = {
    .name = "Blue",
    .main = blue_main
};
//>>>>>>> 8a0975270c4439013eb3433a11c64826ce44e612
