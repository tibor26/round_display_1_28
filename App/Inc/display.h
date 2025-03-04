////////////////////////////////////////////////////////////////////////////////
/// @file    display.h
/// @author  AE TEAM
/// @brief   THIS FILE PROVIDES ALL THE SYSTEM FIRMWARE FUNCTIONS.
////////////////////////////////////////////////////////////////////////////////
/// @attention
///
/// THE EXISTING FIRMWARE IS ONLY FOR REFERENCE, WHICH IS DESIGNED TO PROVIDE
/// CUSTOMERS WITH CODING INFORMATION ABOUT THEIR PRODUCTS SO THEY CAN SAVE
/// TIME. THEREFORE, MINDMOTION SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
/// CONSEQUENTIAL DAMAGES ABOUT ANY CLAIMS ARISING OUT OF THE CONTENT OF SUCH
/// HARDWARE AND/OR THE USE OF THE CODING INFORMATION CONTAINED HEREIN IN
/// CONNECTION WITH PRODUCTS MADE BY CUSTOMERS.
///
/// <H2><CENTER>&COPY; COPYRIGHT MINDMOTION </CENTER></H2>
////////////////////////////////////////////////////////////////////////////////


// Define to prevent recursive inclusion
#ifndef __DISPLAY_H
#define __DISPLAY_H

// Files includes
#include <stdint.h>

void Display_Init(void);
void PutPixel(uint16_t x,uint16_t y,uint16_t color);
void Display_Draw_Bitmap(uint16_t Xstart, uint16_t Xend, uint16_t Ystart, uint16_t Yend, uint8_t *bitmap);
void DispColor(uint16_t color);
void Display_Wait(void);
void Display_Set_Callback(void(*callback)(void));


#endif
