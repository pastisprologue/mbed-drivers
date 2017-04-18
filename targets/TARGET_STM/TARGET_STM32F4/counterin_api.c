#include "counterin_api.h"

#if DEVICE_COUNTERIN

#include "cmsis.h"
#include "pinmap.h"
#include "mbed_error.h"
#include "PeripheralPins.h"

static TIM_HandleTypeDef TimHandle;

void counterin_init(counterin_t* obj, PinName pin)
{
    TIM_SlaveConfigTypeDef sSlaveConfig;
    TIM_MasterConfigTypeDef sMasterConfig;

    obj->cnt = (CNTName)pinmap_peripheral(pin, PinMap_CNT);
    MBED_ASSERT(obj->cnt != (CNTName)NC);

    uint32_t function = pinmap_function(pin, PinMap_CNT);
    MBED_ASSERT(function != (uint32_t)NC);
    obj->channel = STM_PIN_CHANNEL(function);
    obj->inverted = STM_PIN_INVERTED(function);

#if defined(TIM2_BASE)
    if (obj->cnt == CNT_2) __HAL_RCC_TIM2_CLK_ENABLE();
#endif

#if defined(TIM3_BASE)
    if (obj->cnt == CNT_3) __HAL_RCC_TIM3_CLK_ENABLE();
#endif

#if defined(TIM8_BASE)
    if (obj->cnt == CNT_8) __HAL_RCC_TIM8_CLK_ENABLE();
#endif

    // Configure GPIO
    pinmap_pinout(pin, PinMap_CNT);
    obj->pin = pin;

    // Configure Timer
    TimHandle.Instance = (TIM_TypeDef *)(obj->cnt);

    TimHandle.Init.Prescaler = 0;
    TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    TimHandle.Init.Period = 0xFFFF;
    TimHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
    {
        error("Cannot initialize Time Base\n");
    }

    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
    if(obj->channel == 1)
    {
        sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
    }
    else
    {
        sSlaveConfig.InputTrigger = TIM_TS_TI2FP2; 
    }

    if(obj->inverted == 0)
    {
        sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
    }
    else
    {
        sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_FALLING;
    }

    sSlaveConfig.TriggerFilter = 15;
    if (HAL_TIM_SlaveConfigSynchronization(&TimHandle, &sSlaveConfig) != HAL_OK)
    {
        error("Cannot initialize Counter Slave\n");
    }
    

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&TimHandle, &sMasterConfig) != HAL_OK)
    {
        error("Cannot initialize Counter Master\n");
    }
}

void counterin_start( counterin_t* obj )
{
    TimHandle.Instance = (TIM_TypeDef *)(obj->cnt);

    __HAL_TIM_ENABLE(&TimHandle);
}

void counterin_reset( counterin_t* obj )
{
    TimHandle.Instance = (TIM_TypeDef *)(obj->cnt);

    __HAL_TIM_SET_COUNTER(&TimHandle, 0x0000);
}

void counterin_stop(counterin_t* obj)
{
    TimHandle.Instance = (TIM_TypeDef *)(obj->cnt);

    __HAL_TIM_DISABLE(&TimHandle); 
}

uint32_t counterin_read(counterin_t* obj)
{
    TimHandle.Instance = (TIM_TypeDef *)(obj->cnt);

    return (uint32_t) __HAL_TIM_GET_COUNTER(&TimHandle);
}

#endif //DEVICE_COUNTERIN
