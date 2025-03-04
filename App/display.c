////////////////////////////////////////////////////////////////////////////////
/// @file    display.c
/// @author  AE TEAM
/// @brief    In window comparator mode,The transformation results are detected
///           Set the threshold value from 0 to 3V, and connect PB6 and PA0 with
///           jumper cap to see the effect.
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
#define _DISPLAY_C_

#define DISPLAY_ROW  240
#define DISPLAY_COL  240

// display commands
#define DISPLAY_CMD_SLEEP_OUT_MODE  0x11
#define DISPLAY_CMD_COL_ADDR_SET    0x2A
#define DISPLAY_CMD_ROW_ADDR_SET    0x2B
#define DISPLAY_CMD_MEM_WRITE       0x2C
#define DISPLAY_CMD_DISPLAY_ON      0x29

// Files includes
#include "display.h"
#include "mm32_system.h"
#include "hal_dma.h"
#include "hal_rcc.h"
#include "hal_gpio.h"
#include "hal_spi.h"

static uint16_t color_fill = 0;
static void(*spi_callback)(void) = NULL;

// static functions
static void DELAY_Ms(uint32_t delay);
static void Display_Command_Mode(void);
static void Display_Data_Mode(void);
static void SPI2_WriteByte(uint8_t data);
static void SPI2_WriteHalfWord(uint16_t data);
static void SPI2_WriteTwoHalfWord(uint16_t data0, uint16_t data1);
static void Display_WriteComm(uint8_t i);
static void Display_WriteData(uint8_t i);
static void Display_GPIO_Config(void);
static void Display_SPI_Config(u16 spi_baud_div);
static void SPI_LCDInit(u16 spi_baud_div);
static void Display_CS_Low();
static void Display_CS_High();
static void BlockWrite(uint16_t Xstart, uint16_t Xend, uint16_t Ystart, uint16_t Yend);


static void DELAY_Ms(uint32_t delay)
{
    uint32_t start = nTimeOutCnt;
    while ((nTimeOutCnt - start) < delay) ;
}


static void Display_Command_Mode(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_11);
}


static void Display_Data_Mode(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_11);
}


static void SPI2_WriteByte(uint8_t data)
{
    WRITE_REG(SPI2->TDR, data);
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_TXEPT) == 0) ;
}


static void SPI2_WriteHalfWord(uint16_t data)
{
    WRITE_REG(SPI2->ECR, 16);
    WRITE_REG(SPI2->TDR, data);
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_TXEPT) == 0) ;
    WRITE_REG(SPI2->ECR, 8);
}


static void SPI2_WriteTwoHalfWord(uint16_t data0, uint16_t data1)
{
    WRITE_REG(SPI2->ECR, 16);
    WRITE_REG(SPI2->TDR, data0);
    WRITE_REG(SPI2->TDR, data1);
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_TXEPT) == 0) ;
    WRITE_REG(SPI2->ECR, 8);
}


static void Display_WriteComm(uint8_t i)
{
    Display_CS_Low();
    Display_Command_Mode();
    SPI2_WriteByte(i);
    Display_CS_High();
}


static void Display_WriteData(uint8_t i)
{
    Display_CS_Low();
    Display_Data_Mode();
    SPI2_WriteByte(i);
    Display_CS_High();
}


////////////////////////////////////////////////////////////////////////////////
/// @brief  initialize Display MODE1(master)
/// @note   None.
/// @param  None.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
static void Display_GPIO_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_5);
    //spi2_cs    pb12
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    //spi2_sck   pb13
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    //spi2_mosi  pb15
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    //spi2_miso  pb14
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_14;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    //spi2_rst/rs    pb10 pb11
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief  SPI Config
/// @note   None.
/// @param  None.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
static void Display_SPI_Config(u16 spi_baud_div)
{
    SPI_InitTypeDef SPI_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1ENR_SPI2, ENABLE);
    SPI_StructInit(&SPI_InitStruct);
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_32b;
    SPI_InitStruct.SPI_DataWidth = 8;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = (SPI_BaudRatePrescaler_TypeDef)spi_baud_div;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI2, &SPI_InitStruct);
    if (SPI_InitStruct.SPI_BaudRatePrescaler <= 8) {
        exSPI_DataEdgeAdjust(SPI2, SPI_DataEdgeAdjust_FAST);
    }
    SPI_BiDirectionalLineConfig(SPI2, SPI_Direction_Rx);
    SPI_BiDirectionalLineConfig(SPI2, SPI_Direction_Tx);
    NVIC_EnableIRQ(SPI2_IRQn);
    SPI_Cmd(SPI2, ENABLE);
}


static void SPI_DMA_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBENR_DMA1, ENABLE);
    DMA_InitTypeDef dma_init;
    DMA_StructInit(&dma_init);
    dma_init.DMA_PeripheralBaseAddr = (u32)&(SPI2->TDR);
    dma_init.DMA_DIR = DMA_DIR_PeripheralDST;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Disable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMA_Init(DMA1_Channel5, &dma_init);
}


void Display_Init(void)
{
    SPI_LCDInit(2);
    SPI_DMA_Init();
    DispColor(0x0000);
    Display_Wait();
    Display_WriteComm(DISPLAY_CMD_DISPLAY_ON);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief  Modifiable parameter initialization SPI.
/// @note   None.
/// @param  datawidth:data byte length.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
static void SPI_LCDInit(u16 spi_baud_div)
{
    Display_GPIO_Config();
    Display_SPI_Config(spi_baud_div);

    GPIO_SetBits(GPIOB, GPIO_Pin_10); //RST
    DELAY_Ms(10);

    GPIO_ResetBits(GPIOB, GPIO_Pin_10); //RST
    DELAY_Ms(10);

    GPIO_SetBits(GPIOB, GPIO_Pin_10); //RST
    DELAY_Ms(10);

    Display_WriteComm(0xFE);
    Display_WriteComm(0xEF);

    Display_WriteComm(0xEB);
    Display_WriteData(0x14);

    Display_WriteComm(0x84);
    Display_WriteData(0x40);

    Display_WriteComm(0x85);
    Display_WriteData(0xFF);

    Display_WriteComm(0x86);
    Display_WriteData(0xFF);

    Display_WriteComm(0x87);
    Display_WriteData(0xFF);
    Display_WriteComm(0x8E);
    Display_WriteData(0xFF);

    Display_WriteComm(0x8F);
    Display_WriteData(0xFF);

    Display_WriteComm(0x88);
    Display_WriteData(0x0A);

    Display_WriteComm(0x89);
    Display_WriteData(0x21);

    Display_WriteComm(0x8A);
    Display_WriteData(0x00);

    Display_WriteComm(0x8B);
    Display_WriteData(0x80);

    Display_WriteComm(0x8C);
    Display_WriteData(0x01);

    Display_WriteComm(0x8D);
    Display_WriteData(0x01);

    Display_WriteComm(0xB6);
    Display_WriteData(0x00);
    Display_WriteData(0x20);

    Display_WriteComm(0x36);
    Display_WriteData(0x08);

    Display_WriteComm(0x3A);
    Display_WriteData(0x05);


    Display_WriteComm(0x90);
    Display_WriteData(0x08);
    Display_WriteData(0x08);
    Display_WriteData(0x08);
    Display_WriteData(0x08);

    Display_WriteComm(0xBD);
    Display_WriteData(0x06);

    Display_WriteComm(0xBC);
    Display_WriteData(0x00);

    Display_WriteComm(0xFF);
    Display_WriteData(0x60);
    Display_WriteData(0x01);
    Display_WriteData(0x04);

    Display_WriteComm(0xC3);
    Display_WriteData(0x13);
    Display_WriteComm(0xC4);
    Display_WriteData(0x13);

    Display_WriteComm(0xC9);
    Display_WriteData(0x22);

    Display_WriteComm(0xBE);
    Display_WriteData(0x11);

    Display_WriteComm(0xE1);
    Display_WriteData(0x10);
    Display_WriteData(0x0E);

    Display_WriteComm(0xDF);
    Display_WriteData(0x21);
    Display_WriteData(0x0c);
    Display_WriteData(0x02);

    Display_WriteComm(0xF0);
    Display_WriteData(0x45);
    Display_WriteData(0x09);
    Display_WriteData(0x08);
    Display_WriteData(0x08);
    Display_WriteData(0x26);
    Display_WriteData(0x2A);

    Display_WriteComm(0xF1);
    Display_WriteData(0x43);
    Display_WriteData(0x70);
    Display_WriteData(0x72);
    Display_WriteData(0x36);
    Display_WriteData(0x37);
    Display_WriteData(0x6F);


    Display_WriteComm(0xF2);
    Display_WriteData(0x45);
    Display_WriteData(0x09);
    Display_WriteData(0x08);
    Display_WriteData(0x08);
    Display_WriteData(0x26);
    Display_WriteData(0x2A);

    Display_WriteComm(0xF3);
    Display_WriteData(0x43);
    Display_WriteData(0x70);
    Display_WriteData(0x72);
    Display_WriteData(0x36);
    Display_WriteData(0x37);
    Display_WriteData(0x6F);

    Display_WriteComm(0xED);
    Display_WriteData(0x1B);
    Display_WriteData(0x0B);

    Display_WriteComm(0xAE);
    Display_WriteData(0x77);

    Display_WriteComm(0xCD);
    Display_WriteData(0x63);


    Display_WriteComm(0x70);
    Display_WriteData(0x07);
    Display_WriteData(0x07);
    Display_WriteData(0x04);
    Display_WriteData(0x0E);
    Display_WriteData(0x0F);
    Display_WriteData(0x09);
    Display_WriteData(0x07);
    Display_WriteData(0x08);
    Display_WriteData(0x03);

    Display_WriteComm(0xE8);
    Display_WriteData(0x34);

    Display_WriteComm(0x62);
    Display_WriteData(0x18);
    Display_WriteData(0x0D);
    Display_WriteData(0x71);
    Display_WriteData(0xED);
    Display_WriteData(0x70);
    Display_WriteData(0x70);
    Display_WriteData(0x18);
    Display_WriteData(0x0F);
    Display_WriteData(0x71);
    Display_WriteData(0xEF);
    Display_WriteData(0x70);
    Display_WriteData(0x70);

    Display_WriteComm(0x63);
    Display_WriteData(0x18);
    Display_WriteData(0x11);
    Display_WriteData(0x71);
    Display_WriteData(0xF1);
    Display_WriteData(0x70);
    Display_WriteData(0x70);
    Display_WriteData(0x18);
    Display_WriteData(0x13);
    Display_WriteData(0x71);
    Display_WriteData(0xF3);
    Display_WriteData(0x70);
    Display_WriteData(0x70);

    Display_WriteComm(0x64);
    Display_WriteData(0x28);
    Display_WriteData(0x29);
    Display_WriteData(0xF1);
    Display_WriteData(0x01);
    Display_WriteData(0xF1);
    Display_WriteData(0x00);
    Display_WriteData(0x07);

    Display_WriteComm(0x66);
    Display_WriteData(0x3C);
    Display_WriteData(0x00);
    Display_WriteData(0xCD);
    Display_WriteData(0x67);
    Display_WriteData(0x45);
    Display_WriteData(0x45);
    Display_WriteData(0x10);
    Display_WriteData(0x00);
    Display_WriteData(0x00);
    Display_WriteData(0x00);

    Display_WriteComm(0x67);
    Display_WriteData(0x00);
    Display_WriteData(0x3C);
    Display_WriteData(0x00);
    Display_WriteData(0x00);
    Display_WriteData(0x00);
    Display_WriteData(0x01);
    Display_WriteData(0x54);
    Display_WriteData(0x10);
    Display_WriteData(0x32);
    Display_WriteData(0x98);

    Display_WriteComm(0x74);
    Display_WriteData(0x10);
    Display_WriteData(0x85);
    Display_WriteData(0x80);
    Display_WriteData(0x00);
    Display_WriteData(0x00);
    Display_WriteData(0x4E);
    Display_WriteData(0x00);

    Display_WriteComm(0x98);
    Display_WriteData(0x3e);
    Display_WriteData(0x07);



    Display_WriteComm(0x35);
    Display_WriteData(0x00);
    Display_WriteComm(0x21);


    DELAY_Ms(10);
    //--------end gamma setting--------------//

    Display_WriteComm(DISPLAY_CMD_SLEEP_OUT_MODE);
    DELAY_Ms(10);

    //  Display_WriteComm(DISPLAY_CMD_DISPLAY_ON);
    //  DELAY_Ms(120);

    Display_WriteComm(DISPLAY_CMD_MEM_WRITE);
    //  Display_WriteComm(0xF7);     //bist
    //    Display_WriteData(0x20);
    //    Display_WriteData(0x3F);
    //    Display_WriteData(0x00);
    //    Display_WriteData(0x00);

    //    Delay(100000);
    //
    //  Display_WriteComm(0xF7);     //bist
    //    Display_WriteData(0x20);
    //    Display_WriteData(0x00);
    //    Display_WriteData(0x3F);
    //    Display_WriteData(0x00);

    //    Delay(100000);
    //  Display_WriteComm(0xF7);     //bist
    //    Display_WriteData(0x20);
    //    Display_WriteData(0x00);
    //    Display_WriteData(0x00);
    //    Display_WriteData(0x3F);

    //    Delay(100000);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief  Reset internal NSS pins for selected SPI software
/// @note   None.
/// @param  None.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
static void Display_CS_Low()
{
    SPI_CSInternalSelected(SPI2, ENABLE);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief  Reset internal NSS pins for selected SPI software
/// @note   None.
/// @param  None.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
static void Display_CS_High()
{
    SPI_CSInternalSelected(SPI2, DISABLE);
}


static void BlockWrite(uint16_t Xstart, uint16_t Xend, uint16_t Ystart, uint16_t Yend)
{
    Display_Command_Mode();
    SPI2_WriteByte(0x2A);
    Display_Data_Mode();
    SPI2_WriteTwoHalfWord(Xstart, Xend);

    Display_Command_Mode();
    SPI2_WriteByte(0x2B);
    Display_Data_Mode();
    SPI2_WriteTwoHalfWord(Ystart, Yend);

    Display_Command_Mode();
    SPI2_WriteByte(0x2C);
}


void PutPixel(uint16_t x, uint16_t y, uint16_t color)
{
    Display_CS_Low();
    BlockWrite(x, x, y, y);

    Display_Data_Mode();

    SPI2_WriteHalfWord(color);

    Display_CS_High();
}


void Display_Draw_Bitmap(uint16_t Xstart, uint16_t Xend, uint16_t Ystart, uint16_t Yend, uint8_t *bitmap)
{
    Display_CS_Low();
    BlockWrite(Xstart, Xend, Ystart, Yend);

    Display_Data_Mode();

    // set SPI data size and enable interrupt
    WRITE_REG(SPI2->ECR, 16);
    SPI_ClearITPendingBit(SPI2, SPI_IT_TXEPT);
    SPI_ITConfig(SPI2, SPI_IT_TXEPT, ENABLE);

    // set up DMA
    DMA_Cmd(DMA1_Channel5, DISABLE);
    SET_BIT(DMA1_Channel5->CCR, DMA_CCR_MINC); // enable memory increment
    exDMA_SetMemoryAddress(DMA1_Channel5, (uint32_t)bitmap);
    DMA_SetCurrDataCounter(DMA1_Channel5, (Xend - Xstart + 1) * (Yend - Ystart + 1));
    DMA_Cmd(DMA1_Channel5, ENABLE);
    SPI_DMACmd(SPI2, ENABLE);
}


void DispColor(uint16_t color)
{
    Display_CS_Low();
    BlockWrite(0, DISPLAY_COL - 1, 0, DISPLAY_ROW - 1);

    Display_Data_Mode();

    // set SPI data size and enable interrupt
    WRITE_REG(SPI2->ECR, 16);
    SPI_ClearITPendingBit(SPI2, SPI_IT_TXEPT);
    SPI_ITConfig(SPI2, SPI_IT_TXEPT, ENABLE);

    // set up DMA
    DMA_Cmd(DMA1_Channel5, DISABLE);
    color_fill = color;  // save color data in static var
    CLEAR_BIT(DMA1_Channel5->CCR, DMA_CCR_MINC);  // disable memory increment
    exDMA_SetMemoryAddress(DMA1_Channel5, (uint32_t)&color_fill);
    DMA_SetCurrDataCounter(DMA1_Channel5, DISPLAY_COL*DISPLAY_ROW);
    DMA_Cmd(DMA1_Channel5, ENABLE);
    SPI_DMACmd(SPI2, ENABLE);
}


void SPI2_IRQHandler(void)
{
    SPI_ClearITPendingBit(SPI2, SPI_IT_TXEPT);
    if (DMA_GetCurrDataCounter(DMA1_Channel5) == 0)
    {
        SPI_ITConfig(SPI2, SPI_IT_TXEPT, DISABLE);
        SPI_DMACmd(SPI2, DISABLE);
        WRITE_REG(SPI2->ECR, 8);
        Display_CS_High();
        if (spi_callback != NULL)
        {
            spi_callback();
        }
    }
}


void Display_Wait(void)
{
    while ((SPI2->NSSR & SPI_NSSR_NSS) == 0) ;
}


void Display_Set_Callback(void(*callback)(void))
{
    spi_callback = callback;
}