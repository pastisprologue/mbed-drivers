#include "encoderin_api.h"

#if DEVICE_ENCODERIN

#include "cmsis.h"
#include "pinmap.h"
#include "mbed_error.h"
#include "PeripheralPins.h"

#define CHANNEL_NUMBER      5

static TIM_HandleTypeDef TimHandle;
static enc_irq_handler irq_handler;

static uint32_t encoderin_index[CHANNEL_NUMBER];

static uint8_t encoderin_get_irq_index( encoderin_t* obj )
{
    uint8_t irq_index = 0;
    
    switch( obj->enc )
    {
        case ENC_1:
            irq_index = 1;
            break;
        case ENC_3:
            irq_index = 3;
            break;
        case ENC_4:
            irq_index = 4;
            break;
    }

    return irq_index;
}

void encoderin_init( encoderin_t* obj, PinName pinA, PinName pinB, enc_irq_handler handler, uint32_t id )
{
	TIM_Encoder_InitTypeDef sSlaveConfig;
    TIM_MasterConfigTypeDef sMasterConfig;

	obj->enc = (ENCName)pinmap_peripheral(pinA, PinMap_ENC_CHA);
    MBED_ASSERT(obj->enc != (ENCName)NC);

	uint32_t function = pinmap_function(pinA, PinMap_ENC_CHA);
    MBED_ASSERT(function != (uint32_t)NC);

#if defined(TIM1_BASE)
    if (obj->enc == ENC_1) __HAL_RCC_TIM1_CLK_ENABLE();
#endif
#if defined(TIM3_BASE)
    if (obj->enc == ENC_3) __HAL_RCC_TIM3_CLK_ENABLE();
#endif
#if defined(TIM4_BASE)
    if (obj->enc == ENC_4) __HAL_RCC_TIM4_CLK_ENABLE();
#endif

  /* Configure GPIO */
	pinmap_pinout(pinA, PinMap_ENC_CHA);
	obj->pinA = pinA;
	pinmap_pinout(pinB, PinMap_ENC_CHB);
	obj->pinB = pinB;

  /* Configure CH1 & CH2 as Encoder Inputs */
	TimHandle.Instance = (TIM_TypeDef *)(obj->enc);
	TimHandle.Init.Prescaler = 2;
	TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TimHandle.Init.Period = 0xFFFF;
	TimHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	TimHandle.Init.RepetitionCounter = 0;
	sSlaveConfig.EncoderMode = TIM_ENCODERMODE_TI1;
	sSlaveConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
	sSlaveConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sSlaveConfig.IC1Prescaler = TIM_ICPSC_DIV2;
	sSlaveConfig.IC1Filter = 0xF;
	sSlaveConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
	sSlaveConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sSlaveConfig.IC2Prescaler = TIM_ICPSC_DIV2;
	sSlaveConfig.IC2Filter = 0xF;
	if (HAL_TIM_Encoder_Init(&TimHandle, &sSlaveConfig) != HAL_OK)
	{
		error("Cannot initialize the Encoder\n");
	}

  /* Configure Timer Master Mode */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&TimHandle, &sMasterConfig) != HAL_OK)
	{
		error("Cannot intialize Encoder Master Mode\n");
	}	

    TIM_OC_InitTypeDef sConfigOC;

  /* Configure Channel 3 OC */
    sConfigOC.OCMode = TIM_OCMODE_ACTIVE;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_OC_ConfigChannel(&TimHandle, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
    {
        error( "Failed to initialize Output Compare\n" );
    }

  /* Configure Channel 4 OC */
    if (HAL_TIM_OC_ConfigChannel(&TimHandle, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    {
        error( "Failed to initialize Output Compare\n" );
    }

  /* Save for later */
    uint8_t irq_index = encoderin_get_irq_index( obj );
    encoderin_index[irq_index] = id;

    irq_handler = handler;
}

void encoderin_start( encoderin_t* obj )
{
	TimHandle.Instance = (TIM_TypeDef *)(obj->enc);

    HAL_TIM_Encoder_Start( &TimHandle, TIM_CHANNEL_1 );
}

void encoderin_reset( encoderin_t* obj )
{
	TimHandle.Instance = (TIM_TypeDef *)(obj->enc);

    __HAL_TIM_SET_COUNTER(&TimHandle, 0x0000);
}

void encoderin_stop( encoderin_t* obj )
{
	TimHandle.Instance = (TIM_TypeDef *)(obj->enc);

    __HAL_TIM_DISABLE(&TimHandle);
}

uint32_t encoderin_read( encoderin_t* obj )
{
	TimHandle.Instance = (TIM_TypeDef *)(obj->enc);

    return (uint32_t) __HAL_TIM_GET_COUNTER(&TimHandle);
}

static void handle_interrupt( TIM_HandleTypeDef* htim, uint8_t index )
{
  /* Capture compare 3 event */
    if(__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC3) != RESET)
    {
        if(__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_CC3) !=RESET)
        {
            __HAL_TIM_CLEAR_IT(htim, TIM_IT_CC3);
            irq_handler( (uint32_t)encoderin_index[index], IRQ_ALARM1 ); 
        }
    }
  /* Capture compare 4 event */
    if(__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC4) != RESET)
    {
        if(__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_CC4) !=RESET)
        {
            __HAL_TIM_CLEAR_IT(htim, TIM_IT_CC4);
            irq_handler( (uint32_t)encoderin_index[index], IRQ_ALARM2 );
        }
    }
}

static void timer1_irq( void )
{
    TIM_HandleTypeDef htim1;
    htim1.Instance = TIM1;

    handle_interrupt( &htim1, 1 );    
}

static void timer3_irq( void )
{
    TIM_HandleTypeDef htim3;
    htim3.Instance = TIM3;

    handle_interrupt( &htim3, 3 );
}

static void timer4_irq( void )
{
    TIM_HandleTypeDef htim4;
    htim4.Instance = TIM4;

    handle_interrupt( &htim4, 4 );
}

static uint32_t encoderin_get_vector( encoderin_t* obj )
{
    uint32_t vector = (uint32_t)0;

    switch( obj->enc )
    {
        case ENC_1:
            vector = (uint32_t)&timer1_irq;
            break;

        case ENC_3:
            vector = (uint32_t)&timer3_irq;
            break;

        case ENC_4:
            vector = (uint32_t)&timer4_irq;
            break;

        default:
            break;
    }

    return vector;
}

static IRQn_Type encoderin_get_irq_n( encoderin_t* obj )
{
    IRQn_Type irq_n = (IRQn_Type)0;

    switch( obj->enc )
    {
        case ENC_1:
            irq_n = TIM1_CC_IRQn;
            break;
 
        case ENC_3:
            irq_n = TIM3_IRQn;
            break;

        case ENC_4:
            irq_n = TIM4_IRQn;
            break;
        
        default:
            break;
    } 
    
   return irq_n; 
}

void encoderin_set_irq( encoderin_t* obj, enc_irq_event alarm, uint32_t interval )
{
    TimHandle.Instance = (TIM_TypeDef *)(obj->enc);

    TIM_OC_InitTypeDef sConfigOC;
    
    sConfigOC.OCMode = TIM_OCMODE_ACTIVE;
    sConfigOC.Pulse = interval;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    if( alarm == IRQ_ALARM1 )
    {
        if ( HAL_TIM_OC_ConfigChannel(&TimHandle, &sConfigOC, TIM_CHANNEL_3) != HAL_OK )
        {
            error( "Failed to initialize Output Compare\n" );
        }

        if ( HAL_TIM_OC_Start_IT(&TimHandle, TIM_CHANNEL_3) != HAL_OK )
        {
            error( "Failed to start CC Interrupt\n" );
        }
        __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_CC3);
    }
    else
    {
        if (HAL_TIM_OC_ConfigChannel(&TimHandle, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
        {
            error( "Failed to initialize Output Compare\n" );
        }

        if ( HAL_TIM_OC_Start_IT(&TimHandle, TIM_CHANNEL_4) != HAL_OK )
        {
            error( "Failed to start CC Interrupt\n" );
        }
        __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_CC4);
    }

  /* Determine and Set Irq */
    IRQn_Type irq_n = encoderin_get_irq_n( obj );
    uint32_t vector = encoderin_get_vector( obj );
    NVIC_SetVector(irq_n, (uint32_t)vector); 
    NVIC_EnableIRQ(irq_n);
}

void encoderin_irq_enable( encoderin_t* obj )
{
    IRQn_Type irq_n = encoderin_get_irq_n( obj ); 

    NVIC_EnableIRQ( irq_n );
}

void encoderin_irq_disable( encoderin_t* obj )
{
    IRQn_Type irq_n = encoderin_get_irq_n( obj );

    NVIC_DisableIRQ( irq_n );
}

#endif //DEVICE_ENCODERIN
