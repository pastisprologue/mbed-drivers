/** \addtogroup hal */
/** @{*/
/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef TRIGGEREDTIMEOUT_API_H
#define TRIGGEREDTIMEOUT_API_H

#include "device.h"

#if DEVICE_TRIGGEREDTIMEOUT

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IRQ_OVERFLOW,
    IRQ_CC,
} trigger_irq_event;

typedef enum {
	TRG_2 = (int)TIM2_BASE,
	TRG_5 = (int)TIM5_BASE
} TRGName;

const PinMap PinMap_TRG[] = {
	{NC, NC, 0}
};

struct triggeredtimeout_s {
    TRGName trg;
    PinName pin;
    uint8_t channel;
    uint8_t inverted;
};

typedef struct triggeredtimeout_s triggeredtimeout_t;

typedef void (*triggeredtimeout_irq_handler)(uint32_t id, trigger_irq_event event);

void triggeredtimeout_init(triggeredtimeout_t* obj, PinName pin);

void triggeredtimeout_start(triggeredtimeout_t* obj);

void triggeredtimeout_reset(triggeredtimeout_t* obj);

void triggeredtimeout_stop(triggeredtimeout_t* obj);

uint32_t triggeredtimeout_read(triggeredtimeout_t* obj);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif

#endif

/** @}*/
