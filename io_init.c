/*
 * io_init.c
 *
 *  Created on: 26/06/2015
 *      Author: Andre
 */
#include <io_init.h>
void io_init(void)
{

	//Funcao que faz a inicializacao da direcao das portas.
	// |= Saidas (1 saida)
	// &~ Entradas (0 entrada)
	// Setando PORT Correpondente ativa pull up qndo entrada.

	//Transistor que liga o modem.
	DDRC |= (1<<PIN1);
	//LED da placa.
	DDRB |= (1<<PIN5);
	//



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
