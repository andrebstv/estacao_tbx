#ifndef EthernetReset_h
#define EthernetReset_h

#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <string.h>
#include <Arduino.h>
#include <Ethernet.h>
#include <NetEEPROM.h>

#define pgm_uchar(name)   static const prog_uchar name[] PROGMEM

//#define ETHERNETRESET_DEBUG -1
#ifdef ETHERNETRESET_DEBUG
	#define DBG(c) c
#else
	#define DBG(c)
#endif

class EthernetReset
{
	protected:
		EthernetServer* _server;
		char _path[20];
		void stdResponce(char* msg);
		void watchdogReset();
		void stop(void);

	public:
		EthernetClient _client;
		EthernetReset(int port);
		//~EthernetReset();

		void begin(void);
		uint8_t check(void);
};

#endif
