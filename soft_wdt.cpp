/*
 * soft_wdt.cpp
 *
 *  Created on: 25 de nov de 2015
 *      Author: Andre
 */

#include "soft_wdt.h"

SOFT_WDT::SOFT_WDT(uint16_t period) {
	this->periodo = period;
	this->count = 0;
}

void SOFT_WDT::reset() {
	this->count = 0;
}

uint8_t SOFT_WDT::check() {
	if ((this->count) > (this->periodo)) return 0;
	else return 1;
}

void SOFT_WDT::increment() {
	this->count++;
}
