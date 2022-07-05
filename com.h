#ifndef COM_H
#define COM_H

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "Driver_USART.h"
typedef struct{
	char mensaje[128]; //Ponemos 128 de momento(por ejemplo)
	uint8_t tamanoMensaje; //Lo hacemos unsigend
	
}MSGQUEUE_COM_t;

extern int Init_Thread_com(void);
extern int Init_Thread_Pruebas(void);//Hilo de pruebas, borrar para la integración en el programa principal
extern osMessageQueueId_t com_MsgQueue;

#endif
