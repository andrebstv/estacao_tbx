// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef VarioTela3_H_
#define VarioTela3_H_

#include "Arduino.h"
//#include "pins_arduino.h"

//Defaults e libs do compilador
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdio.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

//Perifericos especificos
//#include "WireLib/Wire.h"
extern "C"{
#include "io_init.h"
#include "a2d.h"
}

//Sensores e coisas externas
#include "BMP085.h"
#include "tela.h"
#include "tabela_angulos.h"
#include "RTC/RTClib.h"
//#include "esp.h"
#include "Ethernet.h"
#include "DHT.h"
#include "soft_wdt.h"
#include "server_aux.h"

#include "NewEEPROM.h"
#include "NetEEPROM.h"
#include "SPI.h"
#include "Ethernet.h"
#include <EthernetReset.h> //Para servidor de Update.
#include <FreeRTOS_AVR.h>
#include "SD.h"
#include "wunderground.h"

#define T_PISCA_LED 500
#define T_UPDATE_VARIAVEIS 4000
#define T_UPDATE_SITE 15000
#define T_UPDATE_LCD 1000
#define TASK_ANEMOMETRO_PERIOD 3000
#define TEMPO_MEDICAO_RAJADA 120000
#define OK 1

#define TASK_ANEMOMETRO_PRIORITY (configMAX_PRIORITIES-1)
#define TASK_LCD_PRIORITY (configMAX_PRIORITIES-2)
#define TASK_LED_PRIORITY (configMAX_PRIORITIES-3)
#define TASK_POST_PRIORITY (configMAX_PRIORITIES-5)
#define TASK_PWR_MANAGEMENT_PRIORITY (configMAX_PRIORITIES-4)

#define VERSAO "2.3c"

#define FATOR_DE_ANEMOMETRO 1.56
#define FATOR_DE_FILTRAGEM_VENTO 0.85
extern RTC_DS1307 rtc;

extern File myFile;
#define CS_SDCARD 4

enum estados_wifi {INICIALIZANDO_INT_REDE,CONECTANDO_NA_REDE, CONECTADO ,DESCONECTADO};
typedef enum estados_wifi t_estados_wifi;
enum estados_conectado { MEDINDO, TRANSMITINDO_DADOS };
typedef enum estados_conectado t_estados_c;



#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
void grava_dados_SD (char *palavra, RTC_DS1307 &rtc_log, File &Log);
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project VarioTela3 here




//Do not add code below this line
#endif /* VarioTela3_H_ */
