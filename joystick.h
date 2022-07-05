#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

#define S_PULSE 0x000000001
#define S_TIMEOUT 0x000000002
#define S_PULSE_10 0x000000004 //arriba
#define S_PULSE_11 0x000000008 //derecha
#define S_PULSE_12 0x0000000010 //abajo
#define S_PULSE_14 0x0000000020 //Izquieda
#define S_PULSE_15 0x0000000030 //centro
#define NUM_MAX_MENSAJES 10

typedef enum{ESTADO_IDLE,ESTADO_PULSACION, ESTADO_PULSACION_LARGA, ESTADO_PULSACION_CORTA, ESTADO_ESPERA_PULSACION_LARGA} state_pulsaciones_t;

typedef struct{
	uint8_t Buf[8]; //1 puede valer también
	uint8_t Idx;
}MSGQUEUE_OBJ_t_joystick;

extern osMessageQueueId_t mid_MsgQueue_joystick; 
extern int Init_Thread_Joystick (void);

#endif