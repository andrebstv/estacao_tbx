/*
 * esp.h
 *
 *  Created on: 26/06/2015
 *      Author: Andre
 */

#ifndef ESP_H_
#define ESP_H_


#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdio.h>
#include <avr/wdt.h>

#include "Arduino.h"
extern "C"{
#include "io_init.h"
}
#define OK 1
#define SSID F("APARTAMENTO103")
#define STRING_CONEXAO F("AT+CWJAP=\"apartamento103\",\"pituca20\"\r\n")

extern HardwareSerial Serial;

uint8_t ESP_online();
void ESP_limpa_buffer_serial();
uint8_t ESP_conecta_rede();
uint8_t ESP_posta_dados(float vel_vento, uint16_t direcao_vento,float pressao,float temperatura);

#endif /* ESP_H_ */
