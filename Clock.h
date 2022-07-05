#ifndef CLOCK_H
#define CLOCK_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

extern int Init_Thread_Clock (void);

extern int segundos;
extern int minutos;
extern int horas;
#endif
