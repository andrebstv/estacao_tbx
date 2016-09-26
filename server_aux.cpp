/*
 * server_aux.cpp
 *
 *  Created on: 1 de fev de 2016
 *      Author: Andre
 */
#include "server_aux.h"

//Inicio usando o construtor da anterior antes. Ref:http://www.learncpp.com/cpp-tutorial/114-constructors-and-initialization-of-derived-classes/
AuxServer::AuxServer(int port) : EthernetReset(port)
{
	status_cam = true;
	modo_cam = CAM_AUTO;
}
void AuxServer::check(ST_dados_metereologicos *dadosm, ST_dados_solares *dadossol)
{
	/* 25 is the the maximum command lenth plus
	 * the standart GET and HTTP prefix and postfix */
//	wdt_reset();
	char http_req[strlen(_path) + 35];
	_client = _server->available();
	if(_client) {
		DBG(Serial.println("new reset client");)
		while(_client.connected()) {
			if(_client.available()) {
				char c;
				char* url = http_req;
				while(( c = _client.read()) != '\n'){
					*url = c;
					url++;
				}
				DBG(*url = '\0';)
				url = http_req + 5;
				_client.flush();
				DBG(Serial.println(url);)
				if(!strncmp(url, _path,strlen(_path))) {
					url += (strlen(_path) + 1);
					if(!strncmp(url, "reset", 5)) {
						stdResponce("Estacao reiniciando em 8 segundos");
						watchdogReset();
					} else if(!strncmp(url,"dados", 5)) {
							/*
							 * HEADER HTML
							 */
							_client.println(F("HTTP/1.1 200 OK"));
							_client.println(F("Content-Type: text/html"));
							_client.println(F("Connnection: close"));
							_client.println();
							_client.println(F("<!DOCTYPE HTML>"));
							_client.println(F("<html>"));

							/*
							 * TENSAO DO PAINEL
							 */
							_client.println(F("Dados:<br>Tensao da Bateria:"));
								_client.print(dadossol->tensao_painel,2);
								_client.print(F("<br>Corrente:"));
								_client.print(dadossol->corrente_painel,2);

								/*
								 * HORA LOCAL DO RTC DA ESTACAO
								 */
							_client.print(F("<br>Horario:"));
								_client.print(_rtc->now().hour());
									_client.print(":");
								_client.print(_rtc->now().minute());
									_client.print(":");
								_client.print(_rtc->now().second());

								/*
								 * DADOS DE VENTO
								 */
							_client.print(F("<br>Vento:"));
								_client.print(dadosm->vel_vento,1);
								_client.print(F(" / Direcao:"));
								_client.print(dadosm->direcao_vento);
								_client.print(F("<br>Rajada (2 min):"));
								_client.print(dadosm->rajada,1);
								/*
								 * STATUS DA CAMERA
								 */
							_client.print(F("<br>Camera:"));
								if (this->status_cam)
								{
									_client.print(F("ON"));
								}
								else _client.print(F("OFF"));

								if (this->modo_cam == CAM_AUTO)
								{
									_client.print(F(" [AUTO]"));
								}
								else _client.print(F(" [MANUAL]"));

								/*
								 * FIM DO HEADER HTML
								 */
							_client.println(F("<br><br></html>"));
					}
					/*
					 * AJUSTES DE HORA
					 *
					 */

					 else if(!strncmp(url,"adjhora", 7)) {
						url += 8; //Anda a palavra adjhora + a barra
						char hora[11];
						strncpy(hora,url,8);
						hora[8] = 0;
						DateTime ajuste_hora = DateTime(__DATE__,hora);
						_rtc->adjust(ajuste_hora);
						stdResponce("Hora ajustada");
					 }

					/*
					 * AJUSTES DA CAM
					 */
					 else if(!strncmp(url,"cam_on", 6)) {
						this->liga_cam();
						this->modo_cam = CAM_MAN_ON;
						stdResponce("CAMERA LIGADA - MODO MANUAL");
					 }
					 else if(!strncmp(url,"cam_off", 7)) {
						this->desliga_cam();
						this->modo_cam = CAM_MAN_OFF;
						stdResponce("CAMERA DESLIGADA - MODO MANUAL");
					 }
					 else if(!strncmp(url,"cam_aut", 7)) {
						this->modo_cam = CAM_AUTO;
						stdResponce("CAMERA EM MODO AUTOMATICO - CONTROLE HORARIO<br>ON de 06:00 as 18:00");
					 }



				} else stdResponce("OPCAO INCORRETA");
				break;
			}
		}
		delay(10);
//		wdt_reset();
		_client.flush();
		_client.stop();
		DBG(Serial.println("reset client disonnected");)
	}







}

void AuxServer::liga_cam()
{
	this->status_cam = true;
	CAM_PWR1 = 1;
	CAM_PWR2 = 1;
}

void AuxServer::desliga_cam()
{
	this->status_cam = false;
	CAM_PWR1 = 0;
	CAM_PWR2 = 0;
}

