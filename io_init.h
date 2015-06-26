/*
 * io_init.h
 *
 *  Created on: 10/12/2009
 *      Author: Andre
 */
#include <avr/io.h>
#include "sbit.h"

#define NOVO_PROTOTIPO
//==========================================================================================================//
//													SAIDAS
//==========================================================================================================//
#ifdef NOVO_PROTOTIPO

	//SAIDAS
	#define PWR_ON		SBIT(PORTC,3)
	#define VOLUME			SBIT(PORTD,7)
	#define BUZZER			SBIT(PORTD,0)

	//ENTRADAS
	#define CARREGANDO	SBIT(PINC,1)
	#define BOTAO_ESQ		(fast_adc_10bits(7)>800)
	#define BOTAO_DIR		(!SBIT(PIND,6))

	#define ON 1
	#define OFF 0
	#define BAIXO 1
	#define ALTO 0
#else
	#define botao_esq	SBIT(PINC,3)
	#define botao_dir	SBIT(PINC,2)
#endif


//==========================================================================================================//
//													ENTRADAS
//==========================================================================================================//

//#define ChRot_V_1				SBIT(PIND,3)
#define SENSOR_VEL SBIT(PIND,4)
#define LED SBIT(PORTB,5)

#define SENSOR_DIR_1 (!SBIT(PINB,0))
#define SENSOR_DIR_2 (!SBIT(PINB,2))
#define SENSOR_DIR_3 (!SBIT(PINB,4))
#define SENSOR_DIR_4 (!SBIT(PINB,3))
#define SENSOR_DIR_5 (!SBIT(PINB,1))


void io_init(void)
{

	//Funcao que faz a inicializacao da direcao das portas.
	// |= Saidas (1 saida)
	// &~ Entradas (0 entrada)
	// Setando PORT Correpondente ativa pull up qndo entrada.

	//Pin buzzer do teste
	DDRC |= (1<<PIN0); //Buzzer on.
	//LED da placa.
	DDRB |= (1<<PIN5);
	//Sensor de velocidade e PULL UP
	DDRD &= ~(1<<PIN2);
	PORTD |= (1<<PIN2);
	//Entradas dos sensores de direçao (Biruta Eletronica)
		//Sensor 3 (Arduino D12)
		DDRB &= ~(1<<PIN4);
		PORTB |= (1<<PIN4);
		//Sensor 4 (Arduino D11)
		DDRB &= ~(1<<PIN3);
		PORTB |= (1<<PIN3);
		//Sensor 2 (Arduino D10)
		DDRB &= ~(1<<PIN2);
		PORTB |= (1<<PIN2);
		//Sensor 5 (Arduino D9)
		DDRB &= ~(1<<PIN1);
		PORTB |= (1<<PIN1);
		//Sensor 1 (Arduino D8)
		DDRB &= ~(1<<PIN0);
		PORTB |= (1<<PIN0);

}
