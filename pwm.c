#include "pwm.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_Pwm;                        // thread id

static TIM_HandleTypeDef htim2;
static TIM_OC_InitTypeDef configPWM;

void Thread_Pwm (void *argument);                   // thread function
void inicializarTimer(void);
void inicializarPWM(void);
	
int Init_Thread_Pwm (void) {
 
  tid_Thread_Pwm = osThreadNew(Thread_Pwm, NULL, NULL);
  if (tid_Thread_Pwm == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_Pwm (void *argument) {
	inicializarPWM();

  while (1) {
		osThreadFlagsWait(PWM_BF_S,osFlagsWaitAny,osWaitForever);
		osThreadFlagsClear(PWM_BF_S);
		inicializarTimer();
		//Hay que darle tiempo al timer para pararlo
		osDelay(50); //Vamos a darle 100 ms, lo suficiente para una pulsación
		HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);
		
	}
}

void inicializarPWM(void){
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM2; 
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP ; //Modo alternativo.
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);	
	
}

void inicializarTimer(void){
	

	__HAL_RCC_TIM2_CLK_ENABLE();
	
	htim2.Instance = TIM2;

	htim2.Init.Prescaler = 41; //Ahora la señal es de 1 Mhz
	htim2.Init.Period = 4199; //Señal de 1kHz

	
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;

	
	configPWM.OCMode = TIM_OCMODE_PWM1;
	configPWM.Pulse = CICLO50;
	configPWM.OCPolarity = TIM_OCPOLARITY_HIGH;
	
	
	HAL_TIM_PWM_Init(&htim2);
	HAL_TIM_PWM_ConfigChannel(&htim2, &configPWM, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
}
