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
#define SENSOR SBIT(PIND,4)
#define LED SBIT(PORTB,5)


void io_init(void)
{

	//Funcao que faz a inicializacao da direcao das portas.
	// |= Saidas (1 saida)
	// &~ Entradas (0 entrada)
	// Setando PORT Correpondente ativa pull up qndo entrada.
	DDRB |= (1<<PIN5); //LED
	DDRD &= ~(1<<PIN4);//Sensor e PULL UP
	PORTD |= (1<<PIN4);

}
