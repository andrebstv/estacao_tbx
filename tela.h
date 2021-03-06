/*
 * tela.h
 *
 *  Created on: 26 de nov de 2015
 *      Author: Andre
 */

#ifndef TELA_H_
#define TELA_H_

#include <PCD8544.h>
#include "RTC/RTClib.h"
#include <stdio.h>
#include <stdlib.h>
#include <fontebig.h>
#include <desenhos.h>
#include "wunderground.h"

extern PCD8544 TelaLCD;
extern RTC_DS1307 rtc;

void updateLCD(ST_dados_metereologicos *dadosm, float cpainel, float vpnl);
#endif /* TELA_H_ */
