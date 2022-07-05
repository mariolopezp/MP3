#include "temperatura.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_Temperatura;                        // thread id

extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

//static volatile uint32_t I2C_Event;
static uint8_t slaveAddr = 0x48;
static uint8_t tempAddr = 0x00;
int temperatura;
void conversion(uint8_t MSB, uint8_t LSB);	
void Thread_Temperatura (void *argument);                   // thread function
 
int Init_Thread_Temperatura (void) {
 
  tid_Thread_Temperatura = osThreadNew(Thread_Temperatura, NULL, NULL);
  if (tid_Thread_Temperatura == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_Temperatura (void *argument) {
	uint8_t dataTemperatura[2];
	//Inicializamos el I2C
	I2Cdrv->Initialize(NULL); //Usamos NULL si no necesitamos una función de callback.
	I2Cdrv->PowerControl(ARM_POWER_FULL);
	
	//Configuramos el bus I2C
	I2Cdrv->Control(ARM_I2C_BUS_SPEED,ARM_I2C_BUS_SPEED_STANDARD); //No hay que hacer OWN_ADDRESS, porque somos maestros.
	I2Cdrv->Control(ARM_I2C_BUS_CLEAR,0); 
	
	/* PASOS A SEGUIR*/
	//Primero, enviamos la secuencia de arranque -> Supongo que al registro de temperatura = 0x01
	
	do{
		I2Cdrv->MasterTransmit(slaveAddr,&tempAddr,1,true);  //No hay señal de stop EL PROBLEMA ESTÁ EN EL SEGUNDO PARAMETRO
		//Master transmit genera la consecuencia de start, direcciona al esclavo, si xfer_pending es false, genera stop
	}while(I2Cdrv->GetStatus().busy);
	
	//2º indicamos al esclavo la dirección del registro al que vamos a acceder = 0x00
	
	do{
		I2Cdrv->MasterReceive(slaveAddr,dataTemperatura,2,false);
	}while(I2Cdrv->GetStatus().busy);

  while (1) {
		
		osDelay(1000);
		do{
			I2Cdrv->MasterTransmit(slaveAddr,&tempAddr,1,true);  //No hay señal de stop EL PROBLEMA ESTÁ EN EL SEGUNDO PARAMETRO
			//Master transmit genera la consecuencia de start, direcciona al esclavo, si xfer_pending es false, genera stop
		}while(I2Cdrv->GetStatus().busy);
	
		do{
			I2Cdrv->MasterReceive(tempAddr,dataTemperatura,2,false); 
		}while(I2Cdrv->GetStatus().busy);

		conversion(dataTemperatura[0], dataTemperatura[1]);

			
  }
}

void conversion(uint8_t MSB, uint8_t LSB){
	uint16_t temperaturaLocal;
	temperaturaLocal = (MSB << 3 | LSB>>5);
	if(temperaturaLocal<0x400){
		temperatura = temperaturaLocal*0.125;
	}else{
		temperatura = -(~temperaturaLocal+1)*0.125;
	}
	
}