/*
 * ds4bt.h
 *
 *  Created on: 16 Nov 2016
 *      Author: u564
 */

#ifndef MODULES_DS4_DS4BT_H_
#define MODULES_DS4_DS4BT_H_

extern bool ds4_pairing;

void DS4BT_Start(void);
void DS4BT_Notify_Write(void);
bool DS4BT_IsConnected(void);

#endif /* MODULES_DS4_DS4BT_H_ */
