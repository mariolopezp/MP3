#ifndef LED_H
#define LED_H

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

//Definimos los modos de reproducción

#define REPRODUCIENDO 0x10
#define PAUSA 0x20

typedef struct{
	uint8_t modo_reproduccion;
}MSGQUEUE_LED_t;

extern int Init_Thread_led(void);

extern osMessageQueueId_t led_MsgQueue;

#endif
