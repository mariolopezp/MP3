#ifndef TEMPERATURA_H
#define TEMPERATURA_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "Driver_I2C.h"

#define RegistroTemp 0x00

extern int Init_Thread_Temperatura (void);

extern int temperatura;

#endif
