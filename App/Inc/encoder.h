// Define to prevent recursive inclusion
#ifndef __ENCODER_H
#define __ENCODER_H

#include "hal_gpio.h"

#define ENC_KEY_Port  GPIOA
#define ENC_KEY_Pin   GPIO_Pin_8
#define ENC_A_Port    GPIOA
#define ENC_A_Pin     GPIO_Pin_3
#define ENC_B_Port    GPIOA
#define ENC_B_Pin     GPIO_Pin_2


void encoder_init(void);
uint8_t encoder_key_pressed(void);
int16_t encoder_get_moves(void);
void encoder_check(void);


#endif