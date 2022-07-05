#ifndef PWM_H
#define PWM_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

#define CICLO75 6299
#define CICLO50 2099
#define CICLO25 1049
#define CICLO10 419
#define CICLO5  209

extern int Init_Thread_Pwm(void);

extern osThreadId_t tid_Thread_Pwm;   

#define PWM_BF_S 0x000000001U //A BAJA FRECUENCIA, duración corta

#endif
