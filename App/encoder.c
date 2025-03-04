#include "encoder.h"
#include "hal_rcc.h"
#include "mm32_system.h"

#define ENCODER_STEP      1
#define ENCODER_DEBOUNCE  1
#define ENCODER_REV_DEBOUNCE 100

static bool encA = 0;
static bool encB = 0;
static uint32_t last_step_pos = 0;
static uint32_t last_step_neg = 0;
static int16_t encoder_steps = 0;


void encoder_init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = ENC_KEY_Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ENC_KEY_Port, &GPIO_InitStruct);
    //exGPIO_PinAFConfig(ENC_KEY_Port, ENC_KEY_Pin, 0, GPIO_AF_0);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = ENC_A_Pin | ENC_B_Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_FLOATING;
    GPIO_Init(ENC_A_Port, &GPIO_InitStruct);
    //exGPIO_PinAFConfig(ENC_KEY_Port, ENC_A_Pin, 0, GPIO_AF_0);
    //exGPIO_PinAFConfig(ENC_KEY_Port, ENC_B_Pin, 0, GPIO_AF_0);
    
    encA = GPIO_ReadInputDataBit(ENC_A_Port, ENC_A_Pin);
    encB = GPIO_ReadInputDataBit(ENC_B_Port, ENC_B_Pin);
}


uint8_t encoder_key_pressed(void)
{
    if (GPIO_ReadInputDataBit(ENC_KEY_Port, ENC_KEY_Pin) == 0)
    {
        return 1;
    }
    return 0;
}


void encoder_check(void)
{
    static uint8_t debounce_A = 0;
    static uint8_t debounce_B = 0;

    int16_t new_moves = 0;
    
    bool newA = GPIO_ReadInputDataBit(ENC_A_Port, ENC_A_Pin);
    bool newB = GPIO_ReadInputDataBit(ENC_B_Port, ENC_B_Pin);
    
    if (encA != newA)
    {
        debounce_A++;
        if (debounce_A >= ENCODER_DEBOUNCE)
        {
            debounce_A = 0;
            encA = newA;
            if (newA == newB && (nTimeOutCnt - last_step_neg) >= ENCODER_REV_DEBOUNCE)
            {
                new_moves = ENCODER_STEP;
                last_step_pos = nTimeOutCnt;
            }
        }
    }
    else
    {
        debounce_A = 0;
    }
    if (encB != newB)
    {
        debounce_B++;
        if (debounce_B > ENCODER_DEBOUNCE)
        {
            debounce_B = 0;
            encB = newB;
            if (newA == newB && (nTimeOutCnt - last_step_pos) >= ENCODER_REV_DEBOUNCE)
            {
                new_moves = -ENCODER_STEP;
                last_step_neg = nTimeOutCnt;
            }
        }
    }
    else
    {
        debounce_B = 0;
    }

    encoder_steps += new_moves;
}


int16_t encoder_get_moves(void)
{
    int16_t new_moves = encoder_steps;
    encoder_steps = 0;
    return new_moves;
}
