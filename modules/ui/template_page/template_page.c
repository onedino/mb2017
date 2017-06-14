//<<<<<<< HEAD
///*
// * template_page.c
// *
// *  Created on: 28 Oct 2015
// *      Author: u564
// */
//
//#include "ch.h"
//#include "hal.h"
//#include "gfx.h"
//#include "app.h"
//#include "widgets.h"
//#include "status_bar.h"
//
//
//
//void template_main(void *params){
//  GHandle pageContainer;
//  (void) params;
//
//  font_t font1;
//  font1 = gdispOpenFont("DejaVuSans32_aa");
//  pageContainer = createContainer(0, STATUS_BAR_HEIGHT, GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT - STATUS_BAR_HEIGHT, FALSE);
//  gwinShow(pageContainer);
//  gwinClear(pageContainer);
//  gdispDrawString(0, STATUS_BAR_HEIGHT, "404 Not Found", font1, HTML2COLOR(0x09180A));
//
//  ui_event *evt = NULL;
//
//  while(TRUE){
//
//    if(chMBFetch(&app_mb, (msg_t*)(&evt), TIME_INFINITE) == MSG_OK){
//
//      switch(evt->type){
//        case UI_DS4_BUTTON:
//        case UI_INPUT_BUTTON:
//          if(evt->data.button_state == UI_BUTTON_BACK){
//            return;
//          } else if(evt->data.button_state == UI_BUTTON_ENTER){
//
//          } else {
//
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
//  }
//
//}
//
//
//application template = {
//    .name = "Template",
//    .main = template_main
//};
//=======
/*
 * template_page.c
 *
 *  Created on: 28 Oct 2015
 *      Author: u564
 */

#include "ch.h"
#include "hal.h"
#include "gfx.h"
#include "app.h"
#include "widgets.h"
#include "status_bar.h"



void template_main(void *params){
  GHandle pageContainer;
  (void) params;

  font_t font1;
  font1 = gdispOpenFont("DejaVuSans32_aa");
  pageContainer = createContainer(0, STATUS_BAR_HEIGHT, GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT - STATUS_BAR_HEIGHT, FALSE);
  gwinShow(pageContainer);
  gwinClear(pageContainer);
  gdispDrawString(0, STATUS_BAR_HEIGHT, "404 Not Found", font1, HTML2COLOR(0x09180A));

  ui_event *evt = NULL;

  while(TRUE){

    if(chMBFetch(&app_mb, (msg_t*)(&evt), TIME_INFINITE) == MSG_OK){

      switch(evt->type){
        case UI_DS4_BUTTON:
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
  }

}


application template = {
    .name = "Template",
    .main = template_main
};
//>>>>>>> 8a0975270c4439013eb3433a11c64826ce44e612
