#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "Clock.h"
#include "lcd.h"
#include "joystick.h"
#include "vol.h"
#include "com.h"
#include "pwm.h"
#include "temperatura.h"
#include "led.h"
#include "mp3.h"

#define DURACION_ZUMBIDO_PULSO_LARGO 100
#define DURACION_ZUMBIDO_PULSO_CORTO 50

extern int Init_Thread_Principal (void);

typedef enum{ESTADO_REPOSO,ESTADO_REPRODUCCION} state_t;

typedef enum{ESTADO_INICIAL, ESTADO_PAUSA, ESTADO_PULSACION_IZQUIERDA,ESTADO_PULSACION_DERECHA, 
	ESTADO_PULSACION_ABAJO,ESTADO_PULSACION_ARRIBA,ESTADO_PULSACION_DERECHA_LARGA,
	ESTADO_PULSACION_IZQUIERDA_LARGA, ESTADO_REPRODUCIENDO} state_reproduccion_t;

#endif
