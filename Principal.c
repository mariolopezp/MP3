#include "Principal.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_Principal;                        // thread id


void Thread_Principal (void *argument);                   // thread function

/********************************************************************************
         FUNCIONES PRIVADAS DEL HILO PRINCIPAL
*********************************************************************************/

static void cambiarEstado(void);
static void modoReposo(void);
static void modoReproduccion(void);
static int gestionPulsacionesModoReproduccion(int estadoActual);
static void generarComandoMP3(int comando);
static void generarComandoCom_PC(void);
static void TimerTrepr_Callback(void);
static void gestionar_LCD_Modo_Reproduccion(void);
static void gestionarVolumen(void);
static void modoReproduccionContinua(void);

/********************************************************************************
         VARIABLES GLOBALES DEL HILO PRINCIPAL
*********************************************************************************/

static int numeroCancionActual;//cancion
static int numeroCarpeta;//fichero

static float voltajeNormalizadoVolumen;
static uint8_t volumen;
static uint8_t volumenCambios;

static int numeroIteracionModoReproduccion;
static int CuentaIteracionReproduccion;

static int cancionInicial;
static int pausa;
static int play;
static int estadoAnterior;
static int reproduccionContinua;

static int tiempoReproduccion;
static int segundosRepr;
static int minutosRepr;
static osTimerId_t tim_trepr_id;
static uint32_t exec;

static int finBucleAutomataPrincipal;

//Estados de los dos autómatas que vamos a manejar.
static state_t state;
static state_reproduccion_t state_reproduccion;



/*********************************************************************************
		Colas de mensajes de los módulos que vamos a comunicar con el 
		hilo principal.
**********************************************************************************/

static MSGQUEUE_OBJ_t_joystick msg_joystick; //Comunicacion con el joystick
osMessageQueueId_t mid_MsgQueue_joystick;
static 	osStatus_t statusColaJoystick;

static MSGQUEUE_OBJ_t_lcd msg_lcd; //Comunicación con el lcd
osMessageQueueId_t mid_MsgQueue_lcd; //La cola que vamos a enviar

static MSGQUEUE_MP3_t msg_mp3;  //Comunicación con el MP3
osMessageQueueId_t mp3_MsgQueue; //La cola que vamos a enviar

static MSGQUEUE_COM_t msg_com; //Comunicación con el PC
osMessageQueueId_t com_MsgQueue;

static MSGQUEUE_LED_t msg_led; //Comunicacion con el RGB
osMessageQueueId_t led_MsgQueue;


static MSGQUEUE_Volumen_t msg_volumen; //Comunicación con el potenciómetro
static osStatus_t statusColaVolumen;
	
/************************************************
	VARIABLES GLOBALES DE OTROS HILOS
	
	no hace falta declararlas, porque ya las incluimos
	en el principal.h
	***********************************************/




/*************************************************************
						HILO PRINCIPAL
**************************************************************/



int Init_Thread_Principal (void) {
 
  tid_Thread_Principal = osThreadNew(Thread_Principal, NULL, NULL);
  if (tid_Thread_Principal == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_Principal (void *argument) {
	state = ESTADO_REPOSO; //Estado inicial tras un reset
	finBucleAutomataPrincipal = 0;
	numeroCancionActual = 0;
	numeroCarpeta = 1;

	/**
	INICIALIZAR LAS COLAS DE MENSAJES A ENVIAR AQUÍ
	**/
	mid_MsgQueue_lcd = osMessageQueueNew(10,sizeof(MSGQUEUE_OBJ_t_lcd), NULL);
	led_MsgQueue = osMessageQueueNew(10,sizeof(MSGQUEUE_LED_t), NULL);
	mp3_MsgQueue = osMessageQueueNew(10, sizeof(MSGQUEUE_MP3_t), NULL);
	com_MsgQueue = osMessageQueueNew(10, sizeof(MSGQUEUE_COM_t), NULL);
	
	while(1){

		switch(state){
			case ESTADO_REPOSO:
				finBucleAutomataPrincipal = 0; //Cada vez que se entra a un estado ponemos el bucle a cero
				generarComandoMP3(SLEEP_MODE); 
				while(finBucleAutomataPrincipal==0){
					modoReposo();	
					cambiarEstado();
				}
				numeroIteracionModoReproduccion = 0;
				break;
			case ESTADO_REPRODUCCION:
        generarComandoMP3(RESET);
        osDelay(500);
        generarComandoMP3(SELECT_DEVICE);
        osDelay(200);	

				finBucleAutomataPrincipal = 0;
				estadoAnterior = 0;
				pausa = 0;
				play = 0;
				while(finBucleAutomataPrincipal==0){
					modoReproduccion();
					cambiarEstado();
				}
				break;
			default:
				__NOP();
				break;
		}
	}
	
}

void cambiarEstado(){
	statusColaJoystick = osMessageQueueGet(mid_MsgQueue_joystick, &msg_joystick, 0U, 250U);
	if((msg_joystick.Idx==1)&(msg_joystick.Buf[0]==0x05)&(statusColaJoystick == osOK)){//Condición para pasar de un estado a otro
    osThreadFlagsSet(tid_Thread_Pwm,PWM_BF_S);		//MANDAMOS UN ZUMBIDO
		state_reproduccion = ESTADO_INICIAL;
		finBucleAutomataPrincipal = 1; 
		if(state==ESTADO_REPOSO){
			state = ESTADO_REPRODUCCION;
		}else{
			state = ESTADO_REPOSO;
		}
	}else{
		__NOP();
	}
}

void modoReposo(){	
	
	osDelay(50);
	sprintf(msg_lcd.Buf, "  SBM 2021  T: ""%d" "C", temperatura);
	msg_lcd.Idx = 1; //linea 1
	osMessageQueuePut(mid_MsgQueue_lcd,&msg_lcd,0U,0U);
	
	osDelay(50);
	sprintf(msg_lcd.Buf, "       " "%02d" ":" "%02d" ":" "%02d", horas, minutos, segundos);
	msg_lcd.Idx = 2; //linea 2
	osMessageQueuePut(mid_MsgQueue_lcd,&msg_lcd,0U,0U);	
	
	//GESTION DE LOS LEDS RGB
	msg_led.modo_reproduccion = 0x40; //Para que no se quede brillando.
	osMessageQueuePut(led_MsgQueue, &msg_led, 0U, 0U);
	//GESTION DEL VOLUMEN
	gestionarVolumen();
}

void modoReproduccion(){ //Esto está en bucle.
	osStatus_t status;	
	
	switch(state_reproduccion){
		case ESTADO_INICIAL:
      gestionar_LCD_Modo_Reproduccion();
      CuentaIteracionReproduccion = 0;
			cancionInicial = 0;
			state_reproduccion = gestionPulsacionesModoReproduccion(state_reproduccion);
			estadoAnterior = ESTADO_INICIAL;
			tiempoReproduccion = 0;		//AQUI INICIALIZAMOS EL TIEMPO DE REPRODUCCION.
			if(tim_trepr_id!=NULL){//Si hay un timer creado, se borra.
				osTimerDelete(tim_trepr_id);
			}
			tim_trepr_id = osTimerNew((osTimerFunc_t) &TimerTrepr_Callback, osTimerPeriodic, &exec, NULL);
			gestionarVolumen();
			msg_led.modo_reproduccion = 0x40; //Para que no se quede brillando.
			osMessageQueuePut(led_MsgQueue, &msg_led, 0U, 0U);
      if(numeroIteracionModoReproduccion==0){
				generarComandoMP3(WAKE_UP); 
      }
      numeroIteracionModoReproduccion++;
			break;
			
		case ESTADO_REPRODUCIENDO:
			gestionar_LCD_Modo_Reproduccion();
      CuentaIteracionReproduccion++;
			state_reproduccion = gestionPulsacionesModoReproduccion(state_reproduccion);
			pausa = 0;
			gestionarVolumen();
			if(play<1){
				msg_led.modo_reproduccion = REPRODUCIENDO;
				osMessageQueuePut(led_MsgQueue, &msg_led, 0U, 0U);
				if(cancionInicial==0){
					generarComandoMP3(PLAY_WITH_FOLDER_AND_FILENAME);
					cancionInicial++;
				}else{
					generarComandoMP3(PLAY);
				}
				play++;
			}
			if((tim_trepr_id!=NULL)&(CuentaIteracionReproduccion==1)){
				status = osTimerStart(tim_trepr_id,1000U);
			}
			estadoAnterior = ESTADO_REPRODUCIENDO;
			break;
			
		case ESTADO_PAUSA:
			gestionar_LCD_Modo_Reproduccion();
      CuentaIteracionReproduccion = 0;
			state_reproduccion = gestionPulsacionesModoReproduccion(state_reproduccion);
			play = 0;
			gestionarVolumen();
			generarComandoMP3(PAUSE);
			if(pausa<1){
				msg_led.modo_reproduccion = PAUSA;
				osMessageQueuePut(led_MsgQueue, &msg_led, 0U, 0U);
				pausa++;
			}
			if(tim_trepr_id!=NULL){
				status = osTimerStop(tim_trepr_id);
			}
			estadoAnterior = ESTADO_PAUSA;
			break;
		case ESTADO_PULSACION_IZQUIERDA:
			gestionar_LCD_Modo_Reproduccion();
			CuentaIteracionReproduccion = 0;
			play = 0;
			pausa = 0;		
			if(numeroCancionActual!=0){
				numeroCancionActual = numeroCancionActual - 1;
				tiempoReproduccion = 0;
				generarComandoMP3(PREVIOUS_SONG);
				if(estadoAnterior==ESTADO_INICIAL){
					generarComandoMP3(STOP_PLAY);
				}else{
					__NOP();
				}
			}else{
				__NOP();
			}		
			state_reproduccion = estadoAnterior;
			break;
			
		case ESTADO_PULSACION_DERECHA:
			gestionar_LCD_Modo_Reproduccion();
			CuentaIteracionReproduccion = 0;
			play = 0;
			pausa = 0;
			
			if(numeroCarpeta==1){
        if(numeroCancionActual==0){
					numeroCancionActual++;
					tiempoReproduccion = 0;
					generarComandoMP3(NEXT_SONG);
          if(estadoAnterior==ESTADO_INICIAL){
						generarComandoMP3(STOP_PLAY);
					}else{
						__NOP();
					}
        }else{
          __NOP();
        }
      }else if(numeroCarpeta==2){
         if(numeroCancionActual==2){
          __NOP();
        }else{
          numeroCancionActual++;
					tiempoReproduccion = 0;
					generarComandoMP3(NEXT_SONG);
					if(estadoAnterior==ESTADO_INICIAL){
						generarComandoMP3(STOP_PLAY);
					}else{
						__NOP();
					}
        }
      }else if(numeroCarpeta==3){
				__NOP();
      }else{
        __NOP();
      }
			state_reproduccion = estadoAnterior;
			break;
			
		case ESTADO_PULSACION_ABAJO:
			gestionar_LCD_Modo_Reproduccion();
			CuentaIteracionReproduccion = 0;
			play = 0;
			pausa = 0;
      if(numeroCarpeta==1){
        __NOP();
      }else{
        numeroCarpeta=numeroCarpeta-1;
        numeroCancionActual = 0;
				tiempoReproduccion = 0;
				generarComandoMP3(PLAY_WITH_FOLDER_AND_FILENAME);
				if(estadoAnterior==ESTADO_INICIAL){
					generarComandoMP3(STOP_PLAY);
				}else{
					__NOP();
				}
      }	
      state_reproduccion = estadoAnterior;		
			break;
			
		case ESTADO_PULSACION_ARRIBA:
			gestionar_LCD_Modo_Reproduccion();
			CuentaIteracionReproduccion = 0;
			play = 0;
			pausa = 0;

      if(numeroCarpeta==3){
        __NOP();
      }else{
        numeroCarpeta++;
        numeroCancionActual = 0;
				tiempoReproduccion = 0;
				generarComandoMP3(PLAY_WITH_FOLDER_AND_FILENAME);
				if(estadoAnterior==ESTADO_INICIAL){
					generarComandoMP3(STOP_PLAY);
				}else{
					__NOP();
				}
      }
			state_reproduccion = estadoAnterior;
			break;
			
			case ESTADO_PULSACION_DERECHA_LARGA:
				modoReproduccionContinua();
				gestionarVolumen();
				if(reproduccionContinua==0){
					reproduccionContinua++;
					tiempoReproduccion = 0;
					if(tim_trepr_id!=NULL){
						status = osTimerStart(tim_trepr_id,1000U);
					}
					generarComandoMP3(PLAY_LOOP);
				}
				
				if(gestionPulsacionesModoReproduccion(ESTADO_PULSACION_DERECHA_LARGA)==ESTADO_PULSACION_IZQUIERDA_LARGA){
					state_reproduccion = ESTADO_PULSACION_IZQUIERDA_LARGA;
				}else{
					state_reproduccion = ESTADO_PULSACION_DERECHA_LARGA;
				}

			break;
			case ESTADO_PULSACION_IZQUIERDA_LARGA:
				if(reproduccionContinua>0 ){
					gestionar_LCD_Modo_Reproduccion();
					reproduccionContinua = 0;
					tiempoReproduccion = 0;
					generarComandoMP3(0xAA);
					
				}
				state_reproduccion = estadoAnterior;
			break;
		default:
			__NOP();
		break;
	}	
}

static int gestionPulsacionesModoReproduccion(int estadoActual){
	int estadoFuturo;
	statusColaJoystick = osMessageQueueGet(mid_MsgQueue_joystick, &msg_joystick, 0U, 250U);//El último digito es el tiempo
	if(statusColaJoystick == osOK){
    osThreadFlagsSet(tid_Thread_Pwm,PWM_BF_S);//MANDAMOS UN ZUMBIDO
		if((msg_joystick.Idx==0)&(msg_joystick.Buf[0]==0x05)){//puslacion central corta
			if(estadoActual==ESTADO_REPRODUCIENDO){
				estadoFuturo = ESTADO_PAUSA;
			}else{
				estadoFuturo = ESTADO_REPRODUCIENDO;
			}
		}else if((msg_joystick.Idx==0)&(msg_joystick.Buf[0]==0x01)){//pulsacion no central corta
			estadoFuturo = ESTADO_PULSACION_ARRIBA;
		}else if((msg_joystick.Idx==0)&(msg_joystick.Buf[0]==0x02)){
			estadoFuturo = ESTADO_PULSACION_DERECHA;
		}else if((msg_joystick.Idx==0)&(msg_joystick.Buf[0]==0x03)){
			estadoFuturo = ESTADO_PULSACION_ABAJO;
		}else if((msg_joystick.Idx==0)&(msg_joystick.Buf[0]==0x04)){
			estadoFuturo = ESTADO_PULSACION_IZQUIERDA;
		}else if((msg_joystick.Idx==1)&(msg_joystick.Buf[0]==0x02)){
			estadoFuturo = ESTADO_PULSACION_DERECHA_LARGA;	
    }else if((msg_joystick.Idx==1)&(msg_joystick.Buf[0]==0x04)){
			estadoFuturo = ESTADO_PULSACION_IZQUIERDA_LARGA;
		}else{
			__NOP();
		}
	}else{//Si no hay mensajes que sacar de la cola, seguimos con el estado actual.
		estadoFuturo = estadoActual;
	}
	return estadoFuturo;
}


static void generarComandoMP3(int comando){

	msg_mp3.comando[0] = 0x7E;
	msg_mp3.comando[1] = 0xFF;
	msg_mp3.comando[2] = 0x06;
	//msg_mp3.comando[3] = CMD
	msg_mp3.comando[4] = 0x00; // FBACK Inicialmente no queremos una respuesta.
	//msg_mp3.comando[5] = DAT1	
	//msg_mp3.comando[6] = DAT2		
	msg_mp3.comando[7] = 0xEF;
	
	switch(comando){
		case NEXT_SONG:
			msg_mp3.comando[3] = NEXT_SONG;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x00;
			break;
		case PREVIOUS_SONG:
			msg_mp3.comando[3] = PREVIOUS_SONG;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x00;			
			break;
		case PLAY_WITH_INDEX:
			break;
		case SET_VOLUME:
			msg_mp3.comando[3] = SET_VOLUME;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = volumen;//valor del volumen
			break;
		case SELECT_DEVICE:
			msg_mp3.comando[3] = SELECT_DEVICE;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x02;	
			break;
		case SLEEP_MODE:
			msg_mp3.comando[3] = SLEEP_MODE;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x00;		
			break;
		case WAKE_UP:
			msg_mp3.comando[3] = WAKE_UP;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x00;
			break;
		case RESET:
			msg_mp3.comando[3] = RESET; //0x0C
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x00;
			break;
		case PLAY:
			msg_mp3.comando[3] = PLAY;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x00;
			break;
		case PAUSE:
			msg_mp3.comando[3] = PAUSE;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x00;
			break;
		case PLAY_WITH_FOLDER_AND_FILENAME:
			msg_mp3.comando[3] = PLAY_WITH_FOLDER_AND_FILENAME;
			msg_mp3.comando[5] = numeroCarpeta;//folder
			msg_mp3.comando[6] = numeroCancionActual;//filename	
			break;
		case PLAY_LOOP:
			msg_mp3.comando[3] = PLAY_LOOP;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x01;			
			break;
		case STOP_PLAY:
			msg_mp3.comando[3] = STOP_PLAY;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x00;			
			break;
		case 0xAA: //end loop
			msg_mp3.comando[3] = PLAY_LOOP;
			msg_mp3.comando[5] = 0x00;
			msg_mp3.comando[6] = 0x00;	
			break;
		default:
			__NOP();
			break;
	}
	osMessageQueuePut(mp3_MsgQueue, &msg_mp3, 0U, 0U);
	generarComandoCom_PC();

}

static void TimerTrepr_Callback(){
	tiempoReproduccion++;
}

static void gestionar_LCD_Modo_Reproduccion(){
	
	osDelay(50);
	sprintf(msg_lcd.Buf, "  F:" "%02d" " C:" "%02d" "   Vol:" "%02d" " ", numeroCarpeta, numeroCancionActual, volumen);
	
	msg_lcd.Idx = 1; //linea 1
	osMessageQueuePut(mid_MsgQueue_lcd,&msg_lcd,0U,0U);
	
	osDelay(50);
	minutosRepr= tiempoReproduccion/60;
	segundosRepr=tiempoReproduccion%60;
  if(segundosRepr<10 & minutosRepr <10){
    sprintf(msg_lcd.Buf, "     Trepr:" "0""%d" ":" "0""%d" " " , minutosRepr, segundosRepr);
	}else if(segundosRepr<10){
    sprintf(msg_lcd.Buf, "     Trepr:" "%d" ":" "0""%d " "" , minutosRepr, segundosRepr);
  }else if(minutosRepr <10){
    sprintf(msg_lcd.Buf, "     Trepr:" "0""%d" ":" "%d" " " , minutosRepr, segundosRepr);
  }else{
    sprintf(msg_lcd.Buf, "     Trepr:" "%d" ":" "%d" " " , minutosRepr, segundosRepr);
  }
  msg_lcd.Idx = 2; //linea 2
	osMessageQueuePut(mid_MsgQueue_lcd,&msg_lcd,0U,0U);	
	

}

static void generarComandoCom_PC(void){
	char mensaje[128];
	sprintf(msg_com.mensaje, "%02d" ":" "%02d" ":" "%02d" "--->" "%02x" " " "%02x" " " "%02x" " " "%02x"
		" " "%02x" " " "%02x" " " "%02x" " " "%02x" "\n", horas, minutos, segundos, msg_mp3.comando[0], 
		msg_mp3.comando[1], msg_mp3.comando[2] ,msg_mp3.comando[3] , msg_mp3.comando[4] ,
		msg_mp3.comando[5], msg_mp3.comando[6], msg_mp3.comando[7]);
	
	msg_com.tamanoMensaje = sizeof(msg_com.mensaje);
	osMessageQueuePut(com_MsgQueue, &msg_com, 0U, 0U);
}

static void gestionarVolumen(){
	statusColaVolumen = osMessageQueueGet(mid_MsgQueue_volumen, &msg_volumen, 0U, 0U);
	if(statusColaVolumen == osOK){
		voltajeNormalizadoVolumen = msg_volumen.volumenSeleccionado;
		volumen = 30*voltajeNormalizadoVolumen;	
		if(volumenCambios==volumen){
			__NOP();
		}else{
			generarComandoMP3(SET_VOLUME);
		}
	}
	volumenCambios = volumen;
}

static void modoReproduccionContinua(){
	
	osDelay(50);
	sprintf(msg_lcd.Buf, " Reproduccion continua  " );
	
	msg_lcd.Idx = 1; //linea 1
	osMessageQueuePut(mid_MsgQueue_lcd,&msg_lcd,0U,0U);
	
	osDelay(50);
	minutosRepr= tiempoReproduccion/60;
	segundosRepr=tiempoReproduccion%60;
  
   sprintf(msg_lcd.Buf, "     Trepr:" "%02d" ":" "%02d" , minutosRepr, segundosRepr);

  msg_lcd.Idx = 2; //linea 2
	osMessageQueuePut(mid_MsgQueue_lcd,&msg_lcd,0U,0U);	
	
}