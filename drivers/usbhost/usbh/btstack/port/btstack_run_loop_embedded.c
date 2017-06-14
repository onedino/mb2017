/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

/*
 *  btstack_run_loop_embedded.c
 *
 *  For this run loop, we assume that there's no global way to wait for a list
 *  of data sources to get ready. Instead, each data source has to queried
 *  individually. Calling ds->isReady() before calling ds->process() doesn't 
 *  make sense, so we just poll each data source round robin.
 *
 *  To support an idle state, where an MCU could go to sleep, the process function
 *  has to return if it has to called again as soon as possible
 *
 *  After calling process() on every data source and evaluating the pending timers,
 *  the idle hook gets called if no data source did indicate that it needs to be
 *  called right away.
 *
 */

#include "ch.h"
#include "btstack_run_loop.h"
#include "btstack_run_loop_embedded.h"
#include "hci_transport.h"
#include "btstack_linked_list.h"

#include "btstack_debug.h"

#include <stddef.h> // NULL

static const btstack_run_loop_t btstack_run_loop_embedded;

// the run loop
static btstack_linked_list_t data_sources;
static btstack_linked_list_t timers;

static thread_t *bt_thread;

static uint32_t hal_ms(void){
  return ST2MS(chVTGetSystemTime());
}

/**
 * Add data_source to run_loop
 */
static void btstack_run_loop_embedded_add_data_source(btstack_data_source_t *ds){
    btstack_linked_list_add(&data_sources, (btstack_linked_item_t *) ds);
}

/**
 * Remove data_source from run loop
 */
static int btstack_run_loop_embedded_remove_data_source(btstack_data_source_t *ds){
    return btstack_linked_list_remove(&data_sources, (btstack_linked_item_t *) ds);
}

// set timer
static void btstack_run_loop_embedded_set_timer(btstack_timer_source_t *ts, uint32_t timeout_in_ms){
#ifdef HAVE_EMBEDDED_TIME_MS
    ts->timeout = ST2MS(chVTGetSystemTimeX()) + timeout_in_ms + 1;
#endif
}

/**
 * Add timer to run_loop (keep list sorted)
 */
static void btstack_run_loop_embedded_add_timer(btstack_timer_source_t *ts){
    btstack_linked_item_t *it;
    for (it = (btstack_linked_item_t *) &timers; it->next ; it = it->next){
        // don't add timer that's already in there
        if ((btstack_timer_source_t *) it->next == ts){
           // log_error( "btstack_run_loop_timer_add error: timer to add already in list!");
            return;
        }
        if (ts->timeout < ((btstack_timer_source_t *) it->next)->timeout) {
            break;
        }
    }
    ts->item.next = it->next;
    it->next = (btstack_linked_item_t *) ts;
   // log_info("add ts now %u, timeout %u\n", hal_ms(), ts->timeout);
}

/**
 * Remove timer from run loop
 */
static int btstack_run_loop_embedded_remove_timer(btstack_timer_source_t *ts){
    return btstack_linked_list_remove(&timers, (btstack_linked_item_t *) ts);
}

static void btstack_run_loop_embedded_dump_timer(void){
#ifdef ENABLE_LOG_INFO 
    btstack_linked_item_t *it;
    int i = 0;
    for (it = (btstack_linked_item_t *) timers; it ; it = it->next){
        btstack_timer_source_t *ts = (btstack_timer_source_t*) it;
        log_info("timer %u, timeout %u\n", i, (unsigned int) ts->timeout);
    }
#endif
}

static void btstack_run_loop_embedded_enable_data_source_callbacks(btstack_data_source_t * ds, uint16_t callback_types){
    ds->flags |= callback_types;
}

static void btstack_run_loop_embedded_disable_data_source_callbacks(btstack_data_source_t * ds, uint16_t callback_types){
    ds->flags &= ~callback_types;
}

/**
 * Execute run_loop
 */
static void btstack_run_loop_embedded_execute(void) {

  systime_t timeout_st;
  btstack_timer_source_t *ts;
  btstack_data_source_t *ds;
  btstack_data_source_t *next;
  systime_t now;
  systime_t now_ms;

  while (hci_transport_usb_get_state() == HCI_STATE_READY) {
    // get next timeout
    timeout_st = TIME_INFINITE;
    if (timers) {
      ts = (btstack_timer_source_t *) timers;
      now = chVTGetSystemTime();
      now_ms = ST2MS(now);
      if (ts->timeout < now_ms) {
        // remove timer before processing it to allow handler to re-register with run loop
        btstack_run_loop_remove_timer(ts);
        //log_info("now %u, TM %u\n", now_ms, ts->timeout);
        ts->process(ts);
        continue;
      }
      timeout_st = MS2ST(ts->timeout) - now;
    }

    // pop function call
    if (chEvtWaitAnyTimeout(ALL_EVENTS, timeout_st) != 1) {
      ////log_debug("EVT TMO");
      continue;
    }

// process data sources
    for (ds = (btstack_data_source_t *) data_sources; ds != NULL; ds = next) {
      next = (btstack_data_source_t *) ds->item.next; // cache pointer to next data_source to allow data source to remove itself
      ds->process(ds, ds->flags);
    }
    //chThdSleepMilliseconds(10);
  }
}

static uint32_t btstack_run_loop_embedded_get_time_ms(void){
  return hal_ms();
}


/**
 * trigger run loop iteration
 */
void btstack_run_loop_embedded_trigger(uint8_t evt){
  chEvtSignalI(bt_thread, evt);
}

void btstack_run_loop_embedded_set_thread_pt(thread_t *pt){
  bt_thread = pt;
}

static void btstack_run_loop_embedded_init(void){
    data_sources = NULL;
    timers = NULL;
}

static void btstack_sleep_for_context_switch(void){
  chThdSleepMilliseconds(1);
}

/**
 * Provide btstack_run_loop_embedded instance 
 */
const btstack_run_loop_t * btstack_run_loop_embedded_get_instance(void){
    return &btstack_run_loop_embedded;
}

static const btstack_run_loop_t btstack_run_loop_embedded = {
    &btstack_run_loop_embedded_init,
    &btstack_run_loop_embedded_add_data_source,
    &btstack_run_loop_embedded_remove_data_source,
    &btstack_run_loop_embedded_enable_data_source_callbacks,
    &btstack_run_loop_embedded_disable_data_source_callbacks,
    &btstack_run_loop_embedded_set_timer,
    &btstack_run_loop_embedded_add_timer,
    &btstack_run_loop_embedded_remove_timer,
    &btstack_run_loop_embedded_execute,
    &btstack_run_loop_embedded_dump_timer,
    &btstack_run_loop_embedded_get_time_ms,
};
