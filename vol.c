#include "vol.h"
#include "adc.h" 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_vol;                        // thread id
static MSGQUEUE_Volumen_t msg;
osMessageQueueId_t mid_MsgQueue_volumen;

void Thread_vol (void *argument);                   // thread function
 
int Init_Thread_vol (void) {
 
  tid_Thread_vol = osThreadNew(Thread_vol, NULL, NULL);
  if (tid_Thread_vol == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_vol (void *argument) {
	float value;
  ADC_HandleTypeDef adchandle; //handler definition
	ADC1_pins_F429ZI_config(); //specific PINS configuration
	ADC_Init_Single_Conversion(&adchandle , ADC1); //ADC1 configuration
	mid_MsgQueue_volumen= osMessageQueueNew(10,sizeof(MSGQUEUE_Volumen_t), NULL);
  while (1) {
		osDelay(1000);
	  value=ADC_getVoltage(&adchandle , 10 ); //get values from channel 10->ADC123_IN10
		
		//value=ADC_getVoltage(&adchandle , 13 ); No usar este pone en las diapositivas
		msg.volumenSeleccionado = value/3.3; 
		//Así obtenemos un valor normalizado entre 0 y 1 como volumen 
		//seleccionado, siendo 0 el minimo y 1 el máximo.
		
		osMessageQueuePut(mid_MsgQueue_volumen,&msg, 0U,0U);
		
   
  }
}
