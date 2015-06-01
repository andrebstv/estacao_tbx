
/*
   Descricao: Projeto de um anemometro remoto.

   Usa: Interface Serial


   Sensor:	Sensor hall e imas


        Atualizacoes
        	(13/02/2014)-> Primeiros testes


 */




#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdio.h>
#include <avr/wdt.h>
#include "anemometro.h"
//#include "string.h"
#include "WString.h"
#include "Print.h"
extern "C"{
#include "io_init.h"
#include "a2d.h"
}


//==========================================================================================================//
//										CONFIGURAÇÕES INICIAIS
//==========================================================================================================//
extern HardwareSerial Serial;

void setup()
{

	/*
	 * ========PERIFERICOS E SENSORES ============
	 */
	Serial.begin(9600);
	io_init();
	 adc_init_10b();

}



void loop()
{
	unsigned long tempo = millis();
	uint8_t temp = SENSOR;
	while((SENSOR && (!temp))==0)
	{
		temp = SENSOR;
		if ((millis() - tempo)>3000) break;
		delay(10);
	}
	while((!SENSOR) && temp)
	{
		temp = SENSOR;
		if ((millis() - tempo)>3000) break;
	}
	Serial.println(millis()-tempo);

	 /*
	  * Fim do loop
	  */

}




