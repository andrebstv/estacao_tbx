/*
 * soft_wdt.h
 *
 *  Created on: 25 de nov de 2015
 *      Author: Andre
 */

#ifndef SOFT_WDT_H_
#define SOFT_WDT_H_
#include <Arduino.h>
#include <avr/wdt.h>

class SOFT_WDT
{
public:
	SOFT_WDT(uint16_t period);
	volatile uint16_t periodo;
	void reset();
	void increment();
	uint8_t check();
	uint16_t count;
};



#endif /* SOFT_WDT_H_ */
