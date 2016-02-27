/*
 * server_aux.h
 *
 *  Created on: 1 de fev de 2016
 *      Author: Andre
 */

#ifndef SERVER_AUX_H_
#define SERVER_AUX_H_

#include "EthernetReset.h"
#include "io_init.h"
#include "wunderground.h"
#include "RTC/RTClib.h"

enum ENUM_status_possiveis_camera {CAM_MAN_ON, CAM_MAN_OFF, CAM_AUTO};

class AuxServer : public EthernetReset
{
	private:
		void resposta_dados();
		bool status_cam;
	public:
		ENUM_status_possiveis_camera modo_cam;
		void liga_cam();
		void desliga_cam();
		RTC_DS1307 *_rtc;
		AuxServer(int port);
		void check(ST_dados_metereologicos *dadosm, ST_dados_solares *dadossol);
};

#endif /* SERVER_AUX_H_ */
