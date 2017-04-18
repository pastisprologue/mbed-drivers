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
#ifndef MERE_TRIGGERIN_API_H
#define MERE_TRIGGERIN_API_H

#include "device.h"

#if DEVICE_TRIGGERIN

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IRQ_OVERFLOW,
    IRQ_CC,
} trigger_irq_event;

typedef struct triggerin_s triggerin_t;

typedef void (*triggerin_irq_handler)(uint32_t id, trigger_irq_event event);

void triggerin_init(triggerin_t* obj, PinName pin);

void triggerin_start(triggerin_t* obj);

void triggerin_reset(triggerin_t* obj);

void triggerin_stop(triggerin_t* obj);

uint32_t triggerin_read(triggerin_t* obj);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif

#endif

/** @}*/
