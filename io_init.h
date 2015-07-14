/*
 * io_init.h
 *
 *  Created on: 10/12/2009
 *      Author: Andre
 */
#ifndef IO_INIT_H_
#define IO_INIT_H_

#include <avr/io.h>
#include "sbit.h"

//==========================================================================================================//
//													SAIDAS
//==========================================================================================================//

#define LED SBIT(PORTB,5)
#define PWR_WIFI SBIT(PORTC,1)
#define RESET_ESP SBIT(PORTD,4)

//==========================================================================================================//
//										ENTRADAS
//==========================================================================================================//

#define SENSOR_VEL SBIT(PIND,4)

//Biruta eletronica
#define SENSOR_DIR_1 (!SBIT(PINB,0))
#define SENSOR_DIR_2 (!SBIT(PINB,2))
#define SENSOR_DIR_3 (!SBIT(PINB,4))
#define SENSOR_DIR_4 (!SBIT(PINB,3))
#define SENSOR_DIR_5 (!SBIT(PINB,1))




void io_init(void);
#endif
