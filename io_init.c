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

	//LED da placa.
	DDRB |= (1<<PIN7);

	//Reset do modulo Ethernet
	DDRF |=(1<<PIN0);

	//Modulo da Camera
	DDRK |= (1<<PIN7) + (1<<PIN4);

	//Pino do Anemometro
	DDRG &= ~(1<<PIN0); //Direcao
	PORTG |= (1<<PIN0); //Pullup
	//Entradas dos sensores de direçao (Biruta Eletronica)
	//Sensor 1 - Arduino 31
	DDRC &= ~(1<<PIN6);
	PORTC |= (1<<PIN6);
	//Sensor 2 - Arduino 33
	DDRC &= ~(1<<PIN4);
	PORTC |= (1<<PIN4);
	//Sensor 3 (Arduino 35)
	DDRC &= ~(1<<PIN2);
	PORTC |= (1<<PIN2);
	//Sensor 4 (Arduino 37)
	DDRC &= ~(1<<PIN0);
	PORTC |= (1<<PIN0);
	//Sensor 5 (Arduino 39)
	DDRG &= ~(1<<PIN2);
	PORTG |= (1<<PIN2);

}
