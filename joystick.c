#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "joystick.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_Joystick;                        // thread id
static void Timer_Rebotes_Callback(void *arg);
static void Timer_Pulsacion_Larga_Callback(void *arg);
static void Init_Puertos_Entrada_B(void);
static void Init_Puertos_Entrada_E(void);
static void GestionarPulsaciones(void);
static osStatus_t status_bounce;
static osStatus_t status_long;
static osTimerId_t tim_rebotes_id;
static osTimerId_t tim_pulsacion_larga_id;
static uint32_t exec;

void EXTI15_10_IRQHandler(void);



static MSGQUEUE_OBJ_t_joystick msg_Pulsaciones; 
//osMessageQueueId_t mid_MsgQueue_joystick; 
void Thread_Joystick (void *argument);                   // thread function
 
int Init_Thread_Joystick (void) {
 
  tid_Thread_Joystick = osThreadNew(Thread_Joystick, NULL, NULL);
  if (tid_Thread_Joystick == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_Joystick (void *argument) {
    GestionarPulsaciones();                          
}

void GestionarPulsaciones(){
	state_pulsaciones_t estado = ESTADO_IDLE;
	uint32_t bandera;
	Init_Puertos_Entrada_B();
	Init_Puertos_Entrada_E();

	mid_MsgQueue_joystick = osMessageQueueNew(NUM_MAX_MENSAJES, sizeof(MSGQUEUE_OBJ_t_joystick), NULL);
	tim_rebotes_id = osTimerNew((osTimerFunc_t) &Timer_Rebotes_Callback, osTimerOnce, &exec, NULL);
	tim_pulsacion_larga_id = osTimerNew((osTimerFunc_t) &Timer_Pulsacion_Larga_Callback, osTimerOnce, &exec, NULL);
	
	while(1){
		switch(estado){
			case ESTADO_IDLE:
				osThreadFlagsClear(S_PULSE);
				osThreadFlagsClear(S_PULSE_10);
				osThreadFlagsClear(S_PULSE_11);
				osThreadFlagsClear(S_PULSE_12);
				osThreadFlagsClear(S_PULSE_14);
				osThreadFlagsClear(S_PULSE_15);
				
				osThreadFlagsWait(S_PULSE,osFlagsWaitAny,osWaitForever);
				osThreadFlagsClear(S_PULSE);
				
				osTimerStart(tim_rebotes_id, 50U); //REBOTES
				while(osTimerIsRunning(tim_rebotes_id)){
					if(osThreadFlagsGet()==S_PULSE){
						osTimerStart(tim_rebotes_id, 50U);
					}
				}
				estado = ESTADO_PULSACION;
				break;
				case ESTADO_PULSACION:
					//INICIAMOS EL TIMER PARA VER SI LA PULSACION ES LARGA O NO
				
						osTimerStart(tim_pulsacion_larga_id, 1000U);
				//Esperamos el flanco de bajada durante 1 s:
						osThreadFlagsWait(S_PULSE,osFlagsWaitAny,1000U);//Funciona mejor así
				
						osThreadFlagsClear(S_PULSE);//Quiza es esto, puede que esté limpiando algo que todavía no está puesto
						if(osTimerIsRunning(tim_pulsacion_larga_id)){
								osTimerStart(tim_rebotes_id, 50U);
								while(osTimerIsRunning(tim_rebotes_id)){
								if(osThreadFlagsGet()==S_PULSE){
									osTimerStart(tim_rebotes_id, 50U);
								}
							}
								estado = ESTADO_PULSACION_CORTA;
						}else{
								osTimerStart(tim_rebotes_id, 50U);
								while(osTimerIsRunning(tim_rebotes_id)){
									if(osThreadFlagsGet()==S_PULSE){
										osTimerStart(tim_rebotes_id, 50U);
									}
								}
								estado = ESTADO_PULSACION_LARGA;
							}
					break;	
				case ESTADO_PULSACION_CORTA:
					bandera = osThreadFlagsGet();
					switch(bandera){
						case S_PULSE_10:
							osThreadFlagsClear(S_PULSE_10);
						//Mandamos señal de que se ha producido una señal corta
							msg_Pulsaciones.Buf[0] = 0x01;
							msg_Pulsaciones.Idx = 0;
							osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
						break;
						case S_PULSE_11:
							osThreadFlagsClear(S_PULSE_11);
						//Mandamos señal de que se ha producido una señal corta
							msg_Pulsaciones.Buf[0] = 0x02;
							msg_Pulsaciones.Idx = 0;
							osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
						break;	
						case S_PULSE_12:
							osThreadFlagsClear(S_PULSE_12);
						//Mandamos señal de que se ha producido una señal corta
							msg_Pulsaciones.Buf[0] = 0x03;
							msg_Pulsaciones.Idx = 0;
							osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
						break;	
						case S_PULSE_14:
							osThreadFlagsClear(S_PULSE_14);
						//Mandamos señal de que se ha producido una señal corta
							msg_Pulsaciones.Buf[0] = 0x04;
							msg_Pulsaciones.Idx = 0;
							osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
						break;	
						case S_PULSE_15:
							osThreadFlagsClear(S_PULSE_15);
						//Mandamos señal de que se ha producido una señal corta
							msg_Pulsaciones.Buf[0] = 0x05;
							msg_Pulsaciones.Idx = 0;
							osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
						break;		
						default:
							__NOP();
						break;
					}
					estado = ESTADO_IDLE;
					break;		
			case ESTADO_PULSACION_LARGA:					
				bandera = osThreadFlagsGet();
				//Mandamos señal de que se ha producido una señal corta
				switch(bandera){
					case S_PULSE_10:
						osThreadFlagsClear(S_PULSE_10);//ARRIBA
					//Mandamos señal de que se ha producido una señal corta
						msg_Pulsaciones.Buf[0] = 0x01;
						msg_Pulsaciones.Idx = 1;
						osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
					break;
					case S_PULSE_11:
						osThreadFlagsClear(S_PULSE_11);//DERECHA
					//Mandamos señal de que se ha producido una señal corta
						msg_Pulsaciones.Buf[0] = 0x02;
						msg_Pulsaciones.Idx = 1;
						osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
					break;	
					case S_PULSE_12:
						osThreadFlagsClear(S_PULSE_12);//ABAJO
					//Mandamos señal de que se ha producido una señal corta
						msg_Pulsaciones.Buf[0] = 0x03;
						msg_Pulsaciones.Idx = 1;
						osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
					break;	
					case S_PULSE_14:
						osThreadFlagsClear(S_PULSE_14);//IZQUIERDA
					//Mandamos señal de que se ha producido una señal corta
						msg_Pulsaciones.Buf[0] = 0x04;
						msg_Pulsaciones.Idx = 1;
						osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
					break;	
					case S_PULSE_15:
						osThreadFlagsClear(S_PULSE_15);//CENTRO
					//Mandamos señal de que se ha producido una señal corta
						msg_Pulsaciones.Buf[0] = 0x05;
						msg_Pulsaciones.Idx = 1;
						osMessageQueuePut(mid_MsgQueue_joystick,&msg_Pulsaciones,0U,0U);
					break;		
					default:
						__NOP();
					break;
				}
				estado = ESTADO_ESPERA_PULSACION_LARGA;
				break;
			case ESTADO_ESPERA_PULSACION_LARGA: 
				osThreadFlagsWait(S_PULSE,osFlagsWaitAny,osWaitForever);
				osThreadFlagsClear(S_PULSE);
				estado = ESTADO_IDLE;
				break;
			default:
			 __NOP();
			 break;				
		}
	}
}

void Init_Puertos_Entrada_B(void){
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; //Así producimos una interrupción en flanco de subida y/o bajada.
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 
}

void Init_Puertos_Entrada_E(void){
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOE_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; //Así producimos una interrupción en flanco de subida y/o bajada.
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 
	
}

void Timer_Rebotes_Callback(void *arg){
	osTimerStop(tim_rebotes_id);
}

void Timer_Pulsacion_Larga_Callback(void *arg){
	osTimerStop(tim_pulsacion_larga_id);
}



void EXTI15_10_IRQHandler(void){
	
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
		switch(GPIO_Pin){
			case GPIO_PIN_10:
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE);
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE_10);
			break;
			case GPIO_PIN_11:
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE);
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE_11);
			break;
			case GPIO_PIN_12:
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE);
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE_12);
			break;
			case GPIO_PIN_14:
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE);
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE_14);
			break;
			case GPIO_PIN_15:
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE);
				osThreadFlagsSet(tid_Thread_Joystick,S_PULSE_15);
			break;
	}
}