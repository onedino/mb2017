/*
 * encoder.c
 *
 *  Created on: 12 Oct, 2015
 *      Author: u564
 */

#include "ch.h"
#include "udc.h"
#include "encoder.h"

static unsigned char reset_status;

ENCObj_t encoder1_2 = {.Alive = 0};
ENCObj_t encoder3_4 = {.Alive = 0};

void encoder_setAlive(volatile void * arg){
     ENCObj_t * e = (ENCObj_t *) arg;
     e->Alive = 3;
}

void encoder_decAlive(volatile void * arg){
     ENCObj_t *e = (ENCObj_t * ) arg;
     if ((e->Alive) > 0)
         (e->Alive)--;
}

udc_rx_state_e encoder_resetcount(int encoder_no){
      if (encoder_no == 0){
        UDC_Obj_t object;
        reset_status = 0;
        object.id = 6;
        object.tx_data = (uint8_t*)&reset_status;
        object.tx_len = 1;
        object.rx_len = 0;
        object.rx_data = NULL;
        object.rx_callback = NULL;
        object.arg = NULL;
        return UDC_Poll_Single(&object);
      }else if(encoder_no == 1){
        UDC_Obj_t object;
        reset_status = 1;
        object.id = 6;
        object.tx_data = (uint8_t*)&reset_status;
        object.tx_len = 1;
        object.rx_len = 0;
        object.rx_data = NULL;
        object.rx_callback = NULL;
        object.arg = NULL;
        return UDC_Poll_Single(&object);
      }else if(encoder_no == 2){
        UDC_Obj_t object;
        reset_status = 0;
        object.id = 7;
        object.tx_data = (uint8_t*)&reset_status;
        object.tx_len = 1;
        object.rx_len = 0;
        object.rx_data = NULL;
        object.rx_callback = NULL;
        object.arg = NULL;
        return UDC_Poll_Single(&object);
      }else if(encoder_no == 3){
        UDC_Obj_t object;
        reset_status = 1;
        object.id = 7;
        object.tx_data = (uint8_t*)&reset_status;
        object.tx_len = 1;
        object.rx_len = 0;
        object.rx_data = NULL;
        object.rx_callback = NULL;
        object.arg = NULL;
        return UDC_Poll_Single(&object);
      }
}
