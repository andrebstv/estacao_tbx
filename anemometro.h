// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef VarioTela3_H_
#define VarioTela3_H_
#include "Arduino.h"

//Defaults e libs do compilador
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdio.h>
#include <avr/wdt.h>

//Perifericos especificos
#include "Wire.h"
#include <spi.h>
extern "C"{
#include "io_init.h"
#include "a2d.h"
}

//Sensores e coisas externas
#include "LiquidCrystal_I2C.h"
#include "tabela_angulos.h"
#include "esp.h"
#include "BMP085.h"
#include "Ethernet.h"

#define T_PISCA_LED 500
#define T_UPDATE_VARIAVEIS 3000
#define T_UPDATE_SITE 2000




enum estados_wifi {INICIALIZANDO_INT_REDE,CONECTANDO_NA_REDE, CONECTADO ,DESCONECTADO};
typedef enum estados_wifi t_estados_wifi;
enum estados_conectado { MEDINDO, TRANSMITINDO_DADOS };
typedef enum estados_conectado t_estados_c;

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project VarioTela3 here




//Do not add code below this line
#endif /* VarioTela3_H_ */
