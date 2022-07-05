#include "Clock.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_Clock;                        // thread id

int horas;
int minutos;
int segundos;
static void Led_init();
void Thread_Clock (void *argument);                   // thread function
 
int Init_Thread_Clock (void) {
 
  tid_Thread_Clock = osThreadNew(Thread_Clock, NULL, NULL);
  if (tid_Thread_Clock == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_Clock (void *argument) {
	uint32_t delayTime;
	horas = 0;
	minutos = 0;
	segundos = 0;
	delayTime = 1000U;
	Led_init();
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
  while (1) {
		osDelay(delayTime);
		segundos++;
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
		if(segundos==60){
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
			segundos = 0;
			minutos++;
		}
		if(minutos==60){
			minutos = 0;
			horas++;
		}
		if(horas==24){
			horas = 0;
		}
  }
}

void Led_init(void){
	
	 GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn); 
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	 
}
