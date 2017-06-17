#include "stdint.h"
#include "ch.h"
#include "hal.h"
#include "drivers.h"
#include "gfx.h"
#include "rtt_shell.h"
#include "app.h"
#include "analog.h"
//#include "ps4_usbhost.h"
#include "driving.h"
#include "usage.h"
//#include "ARTracker.h"
#include "RTTLog.h"
#include "ds4.h"
#include "ds4bt.h"
#include "bubbles.h"
#include "hardware_conf.h"
#include "airboard.h"

//static const WDGConfig wdgcfg = {
//  STM32_IWDG_PR_64,
//  STM32_IWDG_RL(5000000)};

#define SDRAM_SIZE  0x1000000
/*
 * Erases the whole SDRAM bank.
 */
static void sdram_bulk_erase(void) {

  volatile uint8_t *p = (volatile uint8_t *)SDRAM_BANK_ADDR;
  volatile uint8_t *end = p + SDRAM_SIZE;
  while (p < end)
    *p++ = 0x0;
}

/*
 * Red LED blinker thread, times are in milliseconds.
 */
//static THD_WORKING_AREA(waThread1, 128);
//static THD_FUNCTION(Thread1, arg) {
//
//  (void)arg;
//  chRegSetThreadName("blinker1");
//  while (TRUE) {
//    palClearPad(GPIOC, GPIOC_LED_R);
//    chThdSleepMilliseconds(400);
//    palSetPad(GPIOC, GPIOC_LED_R);
//    chThdSleepMilliseconds(400);
//  }
//}

/*
 * Blue LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread2, arg) {

  (void)arg;
  chRegSetThreadName("blinker2");
  while (TRUE) {
    palClearPad(GPIOC, GPIOC_LED_B);
    chThdSleepMilliseconds(333);
    palSetPad(GPIOC, GPIOC_LED_B);
    chThdSleepMilliseconds(800);
  }
}

static THD_WORKING_AREA(waUSBHOST, 1024);
static THD_FUNCTION(USBHOST, arg) {

  (void)arg;
  chRegSetThreadName("USB Host Thread");
  while (TRUE) {
    usbhMainLoop(&USBHD2);
    chThdSleepMilliseconds(100);
  }
}

static QEIConfig qeicfg = {
  QEI_MODE_QUADRATURE,
  QEI_BOTH_EDGES,
  QEI_DIRINV_FALSE,
};

/*
 * I2C1 config.
 */
#if BOARD_VERSION == 1
static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};
#elif BOARD_VERSION == 2
static const I2CConfig i2cfg1 = {
    0x20730814,
    0x0300,
    0
};
#endif
static gdispImage myImage;


/*===========================================================================*/
/* Initialization and main thread.                                           */
/*===========================================================================*/
/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  //wdgStart(&WDGD1, &wdgcfg);
  driversInit();

  palClearPad(GPIOD, GPIOD_LCD_DISP);
  palClearPad(GPIOI, GPIOI_LCD_BLCTRL);

  sdram_bulk_erase();

  /*
   * Activates the LCD-related drivers.
   */
  palSetPad(GPIOD, GPIOD_LCD_DISP);
  palSetPad(GPIOI, GPIOI_LCD_BLCTRL);

  palSetPadMode(GPIOB, GPIOB_TIM2_CH4, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(GPIOB, GPIOB_TIM2_CH4);

  palClearPad(GPIOC, GPIOC_LED_R);
  palClearPad(GPIOC, GPIOC_LED_G);


  /*
   * Activates the QEI driver.
   */
  qeiStart(&QEID4, &qeicfg);
  qeiEnable(&QEID4);

  /* EEPROM rework*/
  i2cStart(&I2CD1, &i2cfg1);

  gfxInit();
  gdispImageOpenFile(&myImage, "m2logo.gif");
  gdispImageCache(&myImage);
  gdispImageDraw(&myImage, 40, 180, myImage.width, myImage.height, 0, 0);
  gdispImageClose(&myImage);

  chThdSleepMilliseconds(1000);
  usbhStart(&USBHD2);
  chThdCreateStatic(waUSBHOST, sizeof(waUSBHOST), NORMALPRIO,
                        USBHOST, NULL);
  chThdSleepMilliseconds(100);
  DS4BT_Start();
  DS4_Start();


  chThdSleepMilliseconds(4000);
  /*
   * Creating the blinker threads.
   */
  chThdCreateStatic(waThread2, sizeof(waThread2), LOWPRIO,
                    Thread2, NULL);

  app_init();
  cpu_usage_init();

  /*
   * Shell manager initialization.
   */
//  rtt_shell_init();
//  rtt_shell_start();
//
//  RTTLogObjectInit(&RTT_Log);m[
  chThdSleepMilliseconds(100);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
//  usbDisconnectBus(serusbcfg.usbp);
//  chThdSleepMilliseconds(1000);
//  usbStart(serusbcfg.usbp, &usbcfg);
//  usbConnectBus(serusbcfg.usbp);

  adc_init();
  //ps4_usbhost_init();
  ActivateDriving();
////  artracker_init();

//=======
#if USE_DISTANCESENSOR
  distance_init();	//distance sensor, not quite reliable
#endif

  /*
   * Normal main() thread activity, in this demo it just performs
   * a shell respawn upon its termination.
   */
  while (TRUE) {

//    if (rtt_is_active()) {
//      rtt_shell_create();
     // rtt_shell_wait();               /* Waiting termination.             */
//    }
   chThdSleepMilliseconds(TIME_INFINITE);
  }
}
