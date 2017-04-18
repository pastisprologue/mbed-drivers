#ifndef ENCODER_API_H
#define ENCODER_API_H

#include "device.h"
#include "pinmap.h"

#if DEVICE_ENCODERIN

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IRQ_ALARM1,
    IRQ_ALARM2
} enc_irq_event;

typedef void (*enc_irq_handler)(uint32_t id, enc_irq_event event);

//upon MBED adoption, add to PeripheralNames.h
typedef enum {
    ENC_1 = (int)TIM1_BASE,
    ENC_3 = (int)TIM3_BASE,
    ENC_4 = (int)TIM4_BASE
} ENCName;

//Upon MBED adoption, add to PeripheralPins.c
const PinMap PinMap_ENC_CHA[] = {
	{PE_9, ENC_1, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_TIM1, 1, 0)},
	{PB_4, ENC_3, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF2_TIM3, 1, 0)},
	{PB_6, ENC_4, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF2_TIM4, 1, 0)}
};

const PinMap PinMap_ENC_CHB[] = {
	{PE_11, ENC_1, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_TIM1, 1, 0)},
	{PB_5, ENC_3, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF2_TIM3, 1, 0)},
	{PB_7, ENC_4, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF2_TIM4, 1, 0)}
};

//upon MBED adoption, add to common_objects.h
struct encoderin_s {
    ENCName enc;
    PinName pinA;
	PinName pinB;
};

typedef struct encoderin_s encoderin_t;

void encoderin_init( encoderin_t* obj, PinName pinA, PinName pinB, enc_irq_handler handler, uint32_t id );

void encoderin_start( encoderin_t* obj );

void encoderin_reset( encoderin_t* obj );

void encoderin_stop( encoderin_t* obj );

uint32_t encoderin_read( encoderin_t* obj );

void encoderin_set_irq( encoderin_t* obj, enc_irq_event alarm, uint32_t interval );

void encoderin_irq_enable( encoderin_t* obj );

void encoderin_irq_disable( encoderin_t* obj );

#ifdef __cplusplus
}
#endif

#endif //DEVICE_ENCODERIN

#endif //ENCODER_API_H
