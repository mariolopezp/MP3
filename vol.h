#ifndef __VOL_H
#define __VOL_H

#include "stdint.h"
#include "cmsis_os2.h"  

typedef struct {                                
  float volumenSeleccionado;
} MSGQUEUE_Volumen_t;

extern int Init_Thread_vol(void);

extern osMessageQueueId_t mid_MsgQueue_volumen; //Id de la cola.

#endif
