#include "led.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_led;                        // thread id
 
void Thread_led (void *argument);                   // thread function
void Init_LED_RGB(void);
void Reproduciendo(void);
void Pausa(void);
void Reset(void);
static MSGQUEUE_LED_t msg;
static osStatus_t osStatusQueue;

int Init_Thread_led (void) {
 
  tid_Thread_led = osThreadNew(Thread_led, NULL, NULL);
  if (tid_Thread_led == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_led (void *argument) {
	int modoActual = 0;
	Init_LED_RGB();
	 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,GPIO_PIN_SET);
	 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET);
	 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_SET);
  while (1) {
    osStatusQueue = osMessageQueueGet(led_MsgQueue, &msg, 0U, 0); //No tiene que esperar
		switch(msg.modo_reproduccion){
			case REPRODUCIENDO:
				if(modoActual==REPRODUCIENDO){
					Reproduciendo();
				}else{
					modoActual = REPRODUCIENDO;
					Reset();
					Reproduciendo();
				}
				break;
			case PAUSA:
				if(modoActual==PAUSA){
					Pausa();
				}else{
					modoActual = PAUSA;
					Reset();
					Pausa();
				}
				break;
			default:
				 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,GPIO_PIN_SET);
				 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET);
				 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_SET);				
				break;
		}
	}
}

void Init_LED_RGB(){
	//Puerto D, pines 13(RED), 12(green) y 11(blue)
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct); 
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	
}

void Reproduciendo(){
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_12);
	osDelay(125);
}

void Pausa(){
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_11);
	osDelay(500);	
}

void Reset(){
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_SET);	
}