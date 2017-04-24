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

typedef void (*trg_irq_handler)(uint32_t id);

typedef enum {
	TRG_2 = (int)TIM2_BASE,
	TRG_5 = (int)TIM5_BASE
} TRGName;

const PinMap PinMap_TRG[] = {
    {PA_15, CNT_2, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_TIM2, 1, 0)},
	{NC, NC, 0}
};

struct triggeredtimeout_s {
    TRGName trg;
    PinName pin;
    uint32_t prescaler;
    uint32_t period;
    uint8_t channel;
};

typedef struct triggeredtimeout_s triggeredtimeout_t;

void triggeredtimeout_init(triggeredtimeout_t* obj, PinName pin, trg_irq_handler handler, uint32_t id);

void trigger_set_irq( encoderin_t* obj, uint32_t interval );

void trigger_irq_enable( encoderin_t* obj );

void trigger_irq_disable( encoderin_t* obj );

/**@}*/

#ifdef __cplusplus
}
#endif

#endif

#endif

/** @}*/
