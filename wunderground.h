/*
 * wunderground.h
 *
 *  Created on: 1 de fev de 2016
 *      Author: Andre
 */

#ifndef WUNDERGROUND_H_
#define WUNDERGROUND_H_

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdio.h>

struct ST_dados_metereologicos
{
	float vel_vento;
	int16_t direcao_vento;
	int32_t pressao;
	float temperatura;
	float rajada;
	float umidade;
};
extern ST_dados_metereologicos dados_metereologicos;

struct ST_dados_solares
{
	float tensao_painel;
	float corrente_painel;
};
extern ST_dados_solares dados_solares;


uint8_t WunderWeather_posta_dados(ST_dados_metereologicos *dados_met,float vpainel,float corrente);

#endif /* WUNDERGROUND_H_ */
