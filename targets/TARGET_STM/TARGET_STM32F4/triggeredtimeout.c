#include "triggeredtimeout_api.h"

#if DEVICE_TRIGGEREDTIMEOUT

#include "cmsis.h"
#include "pinmap.h"
#include "mbed_error.h"
#include "PeripheralPins.h"

#define CHANNEL_NUMBER 5

static TIM_HandleTypeDef TimHandle;
static trg_irq_handler irq_handler;

static uint32_t triggeredtimeout_index[CHANNEL_NUMBER];

void triggeredtimeout_init(triggeredtimeout_t* obj, PinName pin, trg_irq_handler handler, uint32_t id)
{
    obj->trg = (TRGName)pinmap_peripheral(pin, PinMap_TRG);
    MBED_ASSERT(obj->trg!= (TRGName)NC);

    uint32_t function = pinmap_function(pin, PinMap_TRG);
    MBED_ASSERT(function != (uint32_t)NC);
    obj->channel = STM_PIN_CHANNEL(function);

#if defined(TIM2_BASE)
    if (obj->trg == TRG_2) {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
#endif

#if defined(TIM3_BASE)
    if (obj->trg == TRG_3) { 
		__HAL_RCC_TIM3_CLK_ENABLE();
	}
#endif

#if defined(TIM8_BASE)
    if (obj->trg == TRG_8) {
		__HAL_RCC_TIM8_CLK_ENABLE();
	}
#endif

    // Configure GPIO
    pinmap_pinout(pin, PinMap_TRG);
    obj->pin = pin;

    irq_handler = handler;
}

void trigger_period(triggeredtimeout_t *obj, float seconds)
{
    trigger_period_us(obj, seconds * 1000000.0f);
}

void trigger_period_ms(triggeredtimeout_t *obj, int ms)
{
    trigger_period_us(obj, ms * 1000);
}

void trigger_period_us(triggeredtimeout_t *obj, int us)
{
    TimHandle.Instance = (TIM_TypeDef *)(obj->trg);
    TIM_SlaveConfigTypeDef sSlaveConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    uint32_t PclkFreq;
    uint32_t APBxCLKDivider;

    __HAL_TIM_DISABLE(&TimHandle);

    // Get clock configuration
    // Note: PclkFreq contains here the Latency (not used after)
    HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &PclkFreq);

    // Get the PCLK and APBCLK divider related to the timer
    switch (obj->trg) {
      
        // APB1 clock
#if defined(TIM2_BASE)
        case TRG_2:
#endif
#if defined(TIM3_BASE)
        case TRG_3:
#endif
            PclkFreq = HAL_RCC_GetPCLK1Freq();
            APBxCLKDivider = RCC_ClkInitStruct.APB1CLKDivider;
            break;
        
        // APB2 clock
#if defined(TIM8_BASE)
        case TRG_8:
#endif
            PclkFreq = HAL_RCC_GetPCLK2Freq();
            APBxCLKDivider = RCC_ClkInitStruct.APB2CLKDivider;
            break;
        default:
            return;
    }

    /* To make it simple, we use to possible prescaler values which lead to:
     * pwm unit = 1us, period/pulse can be from 1us to 65535us
     * or
     * pwm unit = 500us, period/pulse can be from 500us to ~32.76sec
     * Be careful that all the channels of a PWM shares the same prescaler
     */
    if (us >  0xFFFF) {
        obj->prescaler = 500;
    } else {
        obj->prescaler = 1;
    }

    // TIMxCLK = PCLKx when the APB prescaler = 1 else TIMxCLK = 2 * PCLKx
    if (APBxCLKDivider == RCC_HCLK_DIV1)
      TimHandle.Init.Prescaler   = (uint16_t)(((PclkFreq) / 1000000) * obj->prescaler) - 1; // 1 us tick
    else
      TimHandle.Init.Prescaler   = (uint16_t)(((PclkFreq * 2) / 1000000) * obj->prescaler) - 1; // 1 us tick

    if (TimHandle.Init.Prescaler > 0xFFFF)
        error("TRG: out of range prescaler");

    TimHandle.Init.Period        = (us - 1) / obj->prescaler;
    if (TimHandle.Init.Period > 0xFFFF)
        error("TRG: out of range period");

    TimHandle.Init.ClockDivision = 0;
    TimHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;

    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
    {
        error("Cannot initialize Time Base\n");
    }

    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
    if(obj->channel == 1)
    {
        sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
    }
    else
    {
        sSlaveConfig.InputTrigger = TIM_TS_TI2FP2; 
    }

    sSlaveConfig.TriggerFilter = 15;
    if (HAL_TIM_SlaveConfigSynchronization(&TimHandle, &sSlaveConfig) != HAL_OK)
    {
        error("Cannot initialize Trigger Slave\n");
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&TimHandle, &sMasterConfig) != HAL_OK)
    {
        error("Cannot initialize Trigger Master\n");
    }

    // Save for future use
    obj->period = us;

    if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
    {
        error("Cannot Start Timer\n");
    }
}

void trigger_set_irq( encoderin_t* obj, uint32_t interval )
{

}

void trigger_irq_enable( encoderin_t* obj )
{

}

void trigger_irq_disable( encoderin_t* obj )
{

}


#endif //DEVICE_TRIGGEREDTIMEOUT
