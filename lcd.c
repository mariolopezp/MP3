#include "lcd.h"
#include "Arial12x12.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_lcd;   
// thread id
static MSGQUEUE_OBJ_t_lcd msg;
static osStatus_t osStatusQueue;
static uint8_t buffer [512];
static uint8_t positionL1;
static uint8_t positionL2;
static TIM_HandleTypeDef htim7;

extern ARM_DRIVER_SPI Driver_SPI1;
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1; //initialize, uinitialize, powerControl, send, recive, transfer, getDataCount, control...
static int fin;

void Thread_lcd (void *argument);                   // thread function

 
int Init_Thread_lcd (void) {
 
  tid_Thread_lcd = osThreadNew(Thread_lcd, NULL, NULL);
  if (tid_Thread_lcd == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_lcd (void *argument) {
 	LCD_reset(); //El reset ya hace el delay de forma interna.
	LCD_init();

  while (1) {
    printToLCD();
  }
}

void GPIO_Init_Ouptut_HighLevel()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE(); //PA6 LCD_RESET
	__HAL_RCC_GPIOD_CLK_ENABLE(); //PD14 LCD_CS_N
	__HAL_RCC_GPIOF_CLK_ENABLE(); //PF13 LCD_A0

	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_14;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}


void delay(uint32_t n_microsegundos)
{
  __HAL_RCC_TIM7_CLK_ENABLE();
  
  htim7.Instance = TIM7;
	htim7.Init.Prescaler = 41; //reloj a 168Mhz, tim7 a 84MHz, como tenemos máximo 16 bits (65535), por tanto 84Mhz/84=1MHz -> 1us
  htim7.Init.Period = 2*n_microsegundos - 1;
  
	HAL_TIM_Base_Init(&htim7); // configure timer
  HAL_TIM_Base_Start(&htim7); // start timer
    
  while(!__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)){
		__NOP();
	}
	
  __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
  HAL_TIM_Base_Stop(&htim7);
  __HAL_TIM_SET_COUNTER(&htim7,0);
	
}


void LCD_reset()
{
  __SPI1_CLK_ENABLE();
	
  SPIdrv->Initialize(NULL);
  SPIdrv->PowerControl(ARM_POWER_FULL);
  SPIdrv->Control(ARM_SPI_MODE_MASTER|ARM_SPI_CPOL1_CPHA1|ARM_SPI_MSB_LSB|ARM_SPI_DATA_BITS(8), 20000000);
  
	GPIO_Init_Ouptut_HighLevel();
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
  delay(1); 
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
}


void LCD_wr_data(unsigned char data)
{ 
  ARM_SPI_STATUS status;
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);// Seleccionar CS = 0;
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13,GPIO_PIN_SET);// Seleccionar A0 = 1;

  SPIdrv->Send(&data, sizeof(data));
	
  do{
    status = SPIdrv->GetStatus();
  }
  while (status.busy);

  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);// Seleccionar CS = 1;
}


void LCD_wr_cmd(unsigned char cmd)
{
  ARM_SPI_STATUS status;
 
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);// Seleccionar CS = 0;
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13,GPIO_PIN_RESET);// Seleccionar A0 = 0;
	
  SPIdrv->Send(&cmd, sizeof(cmd));

  do{
    status = SPIdrv->GetStatus();
  }
  while (status.busy);

  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);// Seleccionar CS = 1;
}


void LCD_init()
{
	LCD_wr_cmd(0xAE); //El ultimo bit hace que el display se ponga en off(0) o en on(1)
	LCD_wr_cmd(0xA2); //LCD bias set
	LCD_wr_cmd(0xA0); //El ultimo bit hace que el direccionamiento de la ram sea normal(0) o inversa(1)
	LCD_wr_cmd(0xC8); //El bit numero 5 ( el que es un 8 en 0x) hace que el scan en las salidas com es normal(0) o inversa(1)
	LCD_wr_cmd(0x22); //Sleccionan el ratio de la resistencia Rb/Ra, los tres ultimo bits son el ratio.
	LCD_wr_cmd(0x2F); //Power control set, los 3 ultimos bits(a 1 en este caso) seleccionan el modo de operación
	LCD_wr_cmd(0x40); //Selecciona la dirección de inicio del display, los 6 ultimos bits la seleccionan, los dos primeros son fijos (10)
	LCD_wr_cmd(0xAF); //Display en ON, el último bit a 1.
	LCD_wr_cmd(0x81); //Contraste
	LCD_wr_cmd(0x0F); //Valor del contraste
	LCD_wr_cmd(0xA4);//Display all points-> ultimo bit indica si todos los puntos en on(1) o normal(0)
	LCD_wr_cmd(0xA6);//Display normal/reverse -> El ultimo bit indica el modo 0: normal, 1:inverso.
}


void LCD_update()
{
 int i;
 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB0); // Página 0

 for(i=0;i<128;i++){
  LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB1); // Página 1

 for(i=128;i<256;i++){
  LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB2); //Página 2
 
 for(i=256;i<384;i++){
  LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB3); // Pagina 3

 for(i=384;i<512;i++){
  LCD_wr_data(buffer[i]);
 }
}

void symbolToLocalBuffer_L1(uint8_t symbol){
  uint8_t i, value1, value2;
  uint16_t offset=0;
  
  offset=25*(symbol - ' ');
  
  for(i=0; i<12; i++){
    value1=Arial12x12[offset+i*2+1];
    value2=Arial12x12[offset+i*2+2];
    
    buffer[i+positionL1]=value1;
    buffer[i+128+positionL1]=value2;
  }
  
  positionL1=positionL1+Arial12x12[offset];
}


void symbolToLocalBuffer_L2(uint8_t symbol){
  uint8_t i, value1, value2;
  uint16_t offset=0;
  
  offset=25*(symbol - ' ');
  
  for(i=0; i<12; i++){
    value1=Arial12x12[offset+i*2+1];
    value2=Arial12x12[offset+i*2+2];
    
    buffer[i+positionL2+256]=value1;
    buffer[i+128+positionL2+256]=value2;
  }
  
  positionL2=positionL2+Arial12x12[offset];
}


void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
	uint8_t i, value1, value2 ;
	uint16_t offset=0;
	offset=25*(symbol - ' ');
	
	switch(line){
		case 1:
			for(i=0; i<12; i++){
				value1=Arial12x12[offset+i*2+1];
				value2=Arial12x12[offset+i*2+2];
				buffer[i +positionL1] = value1;
				buffer[i+128+positionL1]=value2;
			}
			positionL1 = positionL1+Arial12x12[offset];
			break;
			
		case 2:
			for(i=0; i<12; i++){
				value1=Arial12x12[offset+i*2+1];
				value2=Arial12x12[offset+i*2+2];
				buffer[i+positionL2+256]=value1;
				buffer[i+128 +positionL2+256] = value2;
			}
			positionL2 = positionL2+Arial12x12[offset];
			break;		
		default:
			__NOP();
			break;
		
	}
}
/*
void printToLCD(){
		int i;
		osStatusQueue = osMessageQueueGet(mid_MsgQueue_lcd, &msg, 0U, osWaitForever);
		fin = 0;
		positionL1 = 0;
		positionL2 = 0;

		switch(msg.Idx){
			case 1 : //Escribimos en las dos primeras lineas
				
				for (i = 0; i < 256; i++){
					symbolToLocalBuffer_L1(' ');
				}
				//for(i=0; i<msg.tamanoMensaje; i++){ Hace exactamente lo mismo ambas líneas.
				for(i=0; i<sizeof(msg.Buf); i++){
					if(msg.Buf[i]!=0x00 && fin!= 1){
						symbolToLocalBuffer(msg.Idx, msg.Buf[i]);
						if(msg.Buf[i+1]==0x00){
							fin = 1;
						}
					}
				}
				LCD_update();
				break;
			
			case 2: //Escribimos en las de debajo
				/*
				for (i = 0; i < 156; i++){
					symbolToLocalBuffer_L2(' ');
				}
				for(i=0; i<sizeof(msg.Buf); i++){
					if(msg.Buf[i]!=0x00 && fin!= 1){
						symbolToLocalBuffer(msg.Idx, msg.Buf[i]);
						if(msg.Buf[i+1]==0x00){
							fin = 1;
						}
					}
				}
				LCD_update();
				break;
			
			default:
				__NOP();
				break;
		}
}
*/
void printToLCD(){
		int i;
		osStatusQueue = osMessageQueueGet(mid_MsgQueue_lcd, &msg, 0U, osWaitForever);
		fin = 0;
		positionL1 = 0;
		positionL2 = 0;
	/*
			for (i = 0; i < 256; i++){
				symbolToLocalBuffer_L1(' ');
				symbolToLocalBuffer_L2(' ');
			}
*/
		switch(msg.Idx){
			case 1 : //Escribimos en las dos primeras lineas
				for(i=0; i<sizeof(msg.Buf); i++){
					if(msg.Buf[i]!=0x00 && fin!= 1){
						symbolToLocalBuffer(msg.Idx, msg.Buf[i]);
						if(msg.Buf[i+1]==0x00){
							fin = 1;
						}
					}
				}
				LCD_update();
				break;
			case 2: //Escribimos en las de debajo
				for(i=0; i<sizeof(msg.Buf); i++){
					if(msg.Buf[i]!=0x00 && fin!= 1){
						symbolToLocalBuffer(msg.Idx, msg.Buf[i]);
						if(msg.Buf[i+1]==0x00){
							fin = 1;
						}
					}
				}
				LCD_update();
				break;
			default:
				__NOP();
				break;
		}
}