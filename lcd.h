
#ifndef LCD_H
#define LCD_H
#include "stm32f4xx_hal.h"
#include "stdint.h"
#include "Driver_SPI.h"

#include "cmsis_os2.h"  
#include "stdio.h"

typedef struct {                                
  char Buf[128];
  uint8_t Idx;
	uint8_t tamanoMensaje;
} MSGQUEUE_OBJ_t_lcd;


extern int Init_Thread_lcd(void);
extern osMessageQueueId_t mid_MsgQueue_lcd; //Id de la cola.

//private functions prototype
void GPIO_Init_Ouptut_HighLevel(void);
void delay(uint32_t n_microsegundos);
void LCD_reset(void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);
void LCD_init(void);
void LCD_update(void);
void symbolToLocalBuffer_L1(uint8_t symbol);
void symbolToLocalBuffer_L2(uint8_t symbol);
void symbolToLocalBuffer(uint8_t line, uint8_t symbol);
void printToLCD(void);

#endif