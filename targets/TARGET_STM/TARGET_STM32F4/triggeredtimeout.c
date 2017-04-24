#include "triggeredtimeout_api.h"

#if DEVICE_TRIGGEREDTIMEOUT

#include "cmsis.h"
#include "pinmap.h"
#include "mbed_error.h"
#include "PeripheralPins.h"

#define CHANNEL_NUMBER 5

static TIM_HandleTypeDef TimHandle;
static trg_irq_handler irq_handler;

static uint32_t trg_index[CHANNEL_NUMBER];

static uint8_t trg_get_irq_index( triggeredtimeout_t* obj )
{
    uint8_t irq_index = 0;
    
    switch( obj->trg )
    {
        case TRG_2:
            irq_index = 2;
            break;
    }

    return irq_index;
}

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

    // Configure GPIO
    pinmap_pinout(pin, PinMap_TRG);
    obj->pin = pin;

    uint8_t irq_index = trg_get_irq_index( obj );
    trg_index[irq_index] = id;

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

static void handle_interrupt( TIM_HandleTypeDef* htim, uint8_t index )
{
  /* Overflow event */
    if(__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) != RESET)
    {
        if(__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_UPDATE) !=RESET)
        {
            __HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
            irq_handler( (uint32_t)trg_index[index] ); 
        }

    }
}

static void timer2_irq( void )
{
    TIM_HandleTypeDef htim2;
    htim2.Instance = TIM2;

    handle_interrupt( &htim2, 2 );    
}

static uint32_t trg_get_vector( triggeredtimeout_t* obj )
{
    uint32_t vector = (uint32_t)0;

    switch( obj->trg )
    {
        case TRG_2:
            vector = (uint32_t)&timer2_irq;
            break;

        default:
            break;
    }

    return vector;
}

static IRQn_Type trg_get_irq_n( triggeredtimeout_t* obj )
{
    IRQn_Type irq_n = (IRQn_Type)0;

    switch( obj->trg )
    {
        case TRG_2:
            irq_n = TIM2_IRQn;
            break;
 
        default:
            break;
    } 
    
   return irq_n; 
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
            PclkFreq = HAL_RCC_GetPCLK1Freq();
            APBxCLKDivider = RCC_ClkInitStruct.APB1CLKDivider;
            break;
        
            break;
        default:
            return;
    }

    /* To make it simple, we use to possible prescaler values which lead to:
     * trg unit = 1us, period/pulse can be from 1us to 65535us
     * or
     * trg unit = 500us, period/pulse can be from 500us to ~32.76sec
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
    __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_UPDATE); 

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
    sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_FALLING;
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

  /* Determine and Set Irq */
    IRQn_Type irq_n = trg_get_irq_n( obj );
    uint32_t vector = trg_get_vector( obj );
    NVIC_SetVector(irq_n, (uint32_t)vector); 
    NVIC_EnableIRQ(irq_n);

/*
    if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
    {
        error("Cannot Start Timer\n");
    }
*/
    __HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);
}

void trigger_set_irq( triggeredtimeout_t* obj, uint32_t interval )
{
    trigger_period_us( obj, interval );        
}

void trigger_irq_enable( triggeredtimeout_t* obj )
{
    IRQn_Type irq_n = trg_get_irq_n( obj ); 

    NVIC_EnableIRQ( irq_n );
}

void trigger_irq_disable( triggeredtimeout_t* obj )
{
    IRQn_Type irq_n = trg_get_irq_n( obj );

    NVIC_DisableIRQ( irq_n );
}


#endif //DEVICE_TRIGGEREDTIMEOUT
