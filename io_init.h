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

#define LED SBIT(PORTB,7)
#define WIZNET_RST SBIT(PORTF,0)
#define CAM_PWR1 SBIT(PORTK,7)
#define CAM_PWR2 SBIT(PORTK,4)
//#define PWR_WIFI SBIT(PORTC,1)
//#define RESET_ESP SBIT(PORTD,4)

//==========================================================================================================//
//										ENTRADAS
//==========================================================================================================//

#define SENSOR_VEL (!SBIT(PING,0))

//Biruta eletronica
#define SENSOR_DIR_1 (!SBIT(PINC,6)) //BVD
#define SENSOR_DIR_2 (!SBIT(PINC,4))
#define SENSOR_DIR_3 (!SBIT(PINC,2))
#define SENSOR_DIR_4 (!SBIT(PINC,0))
#define SENSOR_DIR_5 (!SBIT(PING,2))


void io_init(void);
#endif
