/*
 * esp.cpp
 *
 *  Created on: 26/06/2015
 *      Author: Andre
 */
#include <esp.h>

uint8_t ESP_online()
{
	static uint8_t i;
	ESP_limpa_buffer_serial();

	if (i>5)
	{
		//Sequencia de desligamento do modem.
		PWR_WIFI = 0;
		delay(1000);
		PWR_WIFI = 1;
//		i = 0;
		delay(3000);
	}
//	if (i>3)
//	{
//		Serial.print(F("AT+RST\r\n"));
//		delay(5000);
//	}
	ESP_limpa_buffer_serial();
	Serial.print(F("AT\r\n"));
	delay(3000);
	if (Serial.find("OK"))
		{
			i = 0; //Zera contador de erros.
			return OK;
		}
	else
	{
		i++;
		return 0;
	}
}
void ESP_limpa_buffer_serial()
{
	while (Serial.read()!= -1);
}
uint8_t ESP_conecta_rede()
{
	//Seta o modo de operacao
	ESP_limpa_buffer_serial();
	Serial.print(F("AT+CWMODE=1\r\n"));
	delay(3000);
	if (!Serial.find("no change")) return 0;

	//Conecta no roteador
	ESP_limpa_buffer_serial();
	Serial.print(STRING_CONEXAO);
	delay(10000); //Demora mais p/ conectar
	if (!Serial.find("OK")) return 0;

	//Modo de Conexao direto
	Serial.print("AT+CIPMODE=1\r\n");
	delay(2000);

	return OK;
}

uint8_t ESP_posta_dados(float vel_vento, uint16_t direcao_vento,float pressao,float temperatura)
{
	//Conecta no site
	ESP_limpa_buffer_serial();
	Serial.print(F("AT+CIPSTART=\"TCP\",\"weatherstation.wunderground.com\",80\r\n"));
	delay(3000);
	//if (!Serial.find("Linked")) return 0;

	//Starta a conexao para envio
	ESP_limpa_buffer_serial();
	Serial.print(F("AT+CIPSEND\r\n"));
	delay(3000);
	if (!Serial.find(">")) return 0;

	//Starta a conexao para envio
	ESP_limpa_buffer_serial();

	Serial.print(F("GET /weatherstation/updateweatherstation.php?ID=IESPRITO4&PASSWORD=andref&dateutc=now&winddir="));
	Serial.print(direcao_vento);

	Serial.print(F("&windspeedmph="));
	Serial.print(vel_vento/1.6,1); //Conversao para mph, apenas um digito de precisao

	Serial.print(F("&tempf="));
	Serial.print(temperatura,1); //TODO Converter para fareheit

	Serial.print(F("&baromin="));
	Serial.print(pressao*0.0295300586467,1);

	Serial.print(F("&action=updateraw  HTTP/1.0\r\nHost: \r\nConnection: close\r\n\r\n"));

	if (!Serial.find("success")) return 0;

	return OK;
}

