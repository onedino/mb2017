/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
              Copyright (C) 2015 Diego Ismirlian, TISA, (dismirlian (at) google's mail)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


#ifndef USBH_DEBUG_H_
#define USBH_DEBUG_H_

#include "usbh.h"
#include "chprintf.h"
#include "RTTLog.h"

#if DRIVER_USE_USBH

//TODO: Debug is only for USBHD1, make it generic.

#if USBH_DEBUG_ENABLE
#define usbDbgPrintf(format, ...) do {                                   \
  chprintf((BaseSequentialStream*)&RTT_Log, format "\r\n",  ## __VA_ARGS__); \
} while(0)
#define usbDbgPuts(s) do {                                   \
  chprintf((BaseSequentialStream*)&RTT_Log, "%s\r\n", s); \
} while(0)
#define usbDbgInit(host) do {} while(0)
//	void usbDbgPrintf(const char *fmt, ...);
//	void usbDbgPuts(const char *s);
//	void usbDbgInit(USBHDriver *host);
//	void usbDbgReset(void);
//	void usbDbgSystemHalted(void);
#else
#define usbDbgPrintf(fmt, ...) do {} while(0)
#define usbDbgPuts(s) do {} while(0)
#define usbDbgInit(host) do {} while(0)
#define usbDbgReset() do {} while(0)
#define usbDbgSystemHalted() do {} while(0)
#endif

#endif

#endif /* USBH_DEBUG_H_ */
