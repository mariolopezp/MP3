#include "mp3.h"                        // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_mp3;                        // thread id
extern ARM_DRIVER_USART Driver_USART6;
static ARM_DRIVER_USART *USARTdrv = &Driver_USART6;
static MSGQUEUE_MP3_t msg;
static osStatus_t osStatusQueue; //Gracias a static podemos repetir nombres en distintos ficheros.

osMessageQueueId_t mp3_Rx_MsgQueue;
static MSGQUEUE_MP3_t rx_msg;
void Thread_mp3 (void *argument);                   // thread function
void wr_cmd(char data , uint32_t tamano);
void rd_cmd(unsigned char data , uint32_t tamano);

int Init_Thread_mp3 (void) {
 
  tid_Thread_mp3 = osThreadNew(Thread_mp3, NULL, NULL);
  if (tid_Thread_mp3 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_mp3 (void *argument) {
	mp3_Rx_MsgQueue = osMessageQueueNew(10, sizeof(MSGQUEUE_MP3_RX_t), NULL);
 /*
	Inicializamos el USART DRIVER
	*/
	USARTdrv->Initialize(NULL);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS|
										ARM_USART_DATA_BITS_8|
										ARM_USART_PARITY_NONE|
										ARM_USART_STOP_BITS_1|
										ARM_USART_FLOW_CONTROL_NONE, 9600);
	//Esto último es el baud rate
	/*Enable Receiver and Transmitter lines*/
	USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
  while (1) {

		osStatusQueue = osMessageQueueGet(mp3_MsgQueue, &msg, 0U, osWaitForever);
		USARTdrv->Send(msg.comando, sizeof(msg.comando));
		osDelay(100);
		USARTdrv->Receive(rx_msg.comando, sizeof(rx_msg.comando));
		osDelay(100);
		osMessageQueuePut(mp3_Rx_MsgQueue, &rx_msg, 0U, 0U);
	}
}
/*
void wr_cmd(
uint8_t comando[8], uint8_t tamano){
	
	ARM_USART_STATUS status;
	//USARTdrv->Send("\nHOLA ESTE ES MI MENSAJE", 25);
	//Esto es lo que se me imprime
  //USARTdrv->Send(data, tamano);

  do{
    status = USARTdrv->GetStatus();
  }
  while (status.tx_busy);//Cuidado con esto, es distinto para escribir que para leer

}
*/
/*
Vamos a hacer una función para lectura también, para los opcionales puede ser útil*/
/*
void rd_cmd(unsigned char data, uint32_t tamano){
	
	ARM_USART_STATUS status;
 
  USARTdrv->Receive(&data, tamano);

  do{
    status = USARTdrv->GetStatus();
  }
  while (status.rx_busy);//Cuidado con esto, es distinto para escribir que para leer

}
*/