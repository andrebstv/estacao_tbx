/*
 * EthernetReset.cpp - Ariadne Bootloader Reset Server library
 * Copyright (c) 2012 Stylianos Tsampas.  All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "EthernetReset.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

void EthernetReset::stdResponce(char* msg)
{
	_client.println("HTTP/1.1 200 OK");
	_client.println("Content-Type: text/html");
	_client.println("Connnection: close");
	_client.println();
	_client.println("<!DOCTYPE HTML>");
	_client.println("<html>");
	_client.println(msg);
	_client.println("</html>");
}

void EthernetReset::watchdogReset()
{
	delay(10);
	_client.stop();
	wdt_disable();
	cli();
	wdt_enable(WDTO_8S);
	while(1);
}

/******************************************************************************
 * Constructors / Destructors
 ******************************************************************************/
EthernetReset::EthernetReset(int port)
{
	_server =  new EthernetServer(port);
	String path = NetEEPROM.readPass();
	path.toCharArray(_path, 20);
}

/******************************************************************************
 * User API
 ******************************************************************************/

void EthernetReset::begin()
{
//	if(NetEEPROM.netSigIsSet()) {
//		Ethernet.begin(NetEEPROM.readMAC(), NetEEPROM.readIP(), NetEEPROM.readGW(),
//					   NetEEPROM.readGW(), NetEEPROM.readSN());

		_server->begin();
		DBG(
			Serial.print("Server is at ");
			Serial.println(Ethernet.localIP());
			Serial.print("Gw at ");
			Serial.println(NetEEPROM.readGW());
			Serial.print("Password: ");
			Serial.println(_path);)
//	}
}

uint8_t EthernetReset::check()
{	
	/* 25 is the the maximum command lenth plus
	 * the standart GET and HTTP prefix and postfix */
	uint8_t flag = 0;
	wdt_reset();
	char http_req[strlen(_path) + 25];
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
				url = http_req + 7;
				_client.flush();
				DBG(Serial.println(url);)
				if(!strncmp(url, _path,strlen(_path))) {
					url += (strlen(_path) + 1);
					if(!strncmp(url, "reset", 5)) {
						stdResponce("Estacao reiniciando em 2 segundos");
						watchdogReset();
					} else if(!strncmp(url,"dados", 5)) {
						flag = 1;
					}
					 else if(!strncmp(url,"cam_on", 6)) {
						flag = 2;
					 }
					 else if(!strncmp(url,"cam_off", 7)) {
						flag = 3;
					 }
				} else stdResponce("Wrong path");
				break;
			}
		}
		delay(10);
		wdt_reset();
//		_client.stop();
		DBG(Serial.println("reset client disonnected");)
	}
		return flag;
}
