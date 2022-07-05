#include "com.h"                        // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_com;                        // thread id
extern ARM_DRIVER_USART Driver_USART3;
static ARM_DRIVER_USART *USARTdrv = &Driver_USART3;
static MSGQUEUE_COM_t msg;
static osStatus_t osStatusQueue; //Gracias a static podemos repetir nombres en distintos ficheros.


void Thread_com (void *argument);                   // thread function
void wr_cmd(char data , uint32_t tamano);
void rd_cmd(unsigned char data , uint32_t tamano);

int Init_Thread_com (void) {
 
  tid_Thread_com = osThreadNew(Thread_com, NULL, NULL);
  if (tid_Thread_com == NULL) {
    return(-1);
  }
  return(0);
}
 
void Thread_com (void *argument) {
	int i;
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
   //Aquí gestionamos la cola con las peticiones de escritura a TeraTerm		
	osStatusQueue = osMessageQueueGet(com_MsgQueue, &msg, 0U, osWaitForever);
	 USARTdrv->Send(msg.mensaje, msg.tamanoMensaje);
		
	}
}
/*
void wr_cmd(char data, uint32_t tamano){
	
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