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
#ifndef MERE_COUNTERIN_API_H
#define MERE_COUNTERIN_API_H

#include "device.h"

#if DEVICE_COUNTERIN

#ifdef __cplusplus
extern "C" {
#endif

typedef struct counterin_s counterin_t;

void counterin_init(counterin_t* obj, PinName pin);

void counterin_start(counterin_t* obj);

void counterin_reset(counterin_t* obj);

void counterin_stop(counterin_t* obj);

uint32_t counterin_read(counterin_t* obj);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif

#endif

/** @}*/
