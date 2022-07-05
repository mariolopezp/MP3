#ifndef MP3_H
#define MP3_H

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "Driver_USART.h"

//Comandos definidos que vamos a usar (CMD) que corresponden con el byte 4
#define NEXT_SONG 0x01
#define PREVIOUS_SONG 0x02
#define PLAY_WITH_INDEX 0x03
#define VOLUME_UP 0x04
#define VOLUME_DOWN 0x05
#define SET_VOLUME 0x06 
#define SELECT_DEVICE 0x09 
#define SLEEP_MODE 0x0A 
#define WAKE_UP 0x0B
#define RESET 0x0C 
#define PLAY 0x0D 
#define PAUSE 0x0E 
#define PLAY_WITH_FOLDER_AND_FILENAME 0x0F
#define STOP_PLAY 0x16
#define SHUFFLE 0x16
#define PLAY_VOLUME 0x22
#define PLAY_LOOP 0x11
#define END LOOP 0xAA

#define NUMERO_CANCIONES_ENCONTRADAS 0x48
#define CANCION_ACTUAL 0x4C
#define NUMERO_CARPETAS 0x4F
#define VOLUMEN_ACTUAL 0x43
#define FILE_NOT_FOUND 0x40
#define FINSIHED_PLAYING_FILE 0x3D

typedef struct{
	uint8_t comando[8]; //Ponemos 128 de momento(por ejemplo)
	uint8_t Idx; //identificador del mensaje, Si es un 0, es un mensaje de tx
	
}MSGQUEUE_MP3_t; //Mensajes que envíamos al MP3

typedef struct{
	uint8_t comando[8]; //Ponemos 128 de momento(por ejemplo)
	uint8_t Idx; //identificador del mensaje, Si es un 1, es un mensaje de rx
	
}MSGQUEUE_MP3_RX_t; //Mensajes que nos envía el MP3

extern int Init_Thread_mp3(void);
extern int Init_Thread_Pruebas(void);//En este módulo no hace falta hilo de pruebas. Con documentarlas vale
extern osMessageQueueId_t mp3_MsgQueue;
extern osMessageQueueId_t mp3_Rx_MsgQueue;
#endif
