
/*
   Descricao: Projeto de um anemometro remoto.

   Usa: Interface Serial
   	   Modulo ESP8266 para conexao WiFi


   Sensor:	Sensor hall e imas


        Atualizacoes
        	(13/02/2014)-> Primeiros testes
        	(25/06/2015) -> Implementado a mecanica do sensor de velocidade e da biruta eletronica.
        	13/07/2015 -> ESP8266 Atualizado p/ versao nova do firmware. Melhoras significativas no boot.


 */





#include "anemometro.h"

//==========================================================================================================//
//										PROTOTIPOS DE FUNCOES
//==========================================================================================================//

float get_wind_speed();
int16_t le_angulo_biruta();
uint8_t WunderWeather_posta_dados(float vel_vento, uint16_t direcao_vento,float pressao,float temperatura);
//==========================================================================================================//
//										VARIAVEIS GLOBAIS
//==========================================================================================================//
extern HardwareSerial Serial;
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
BMP085   bmp085 = BMP085();
t_estados_wifi estado;
t_estados_c estado_conectado = MEDINDO;

volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;
long lastWindCheck = 0;

float p0 = 101325;                //Pressure at sea level (Pa)
float vel_vento;
uint16_t direcao_vento;
long pressao = 101325;
int32_t temperatura;
uint8_t conta_erros, conta_conex_wifi;
uint8_t tensao_saida;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

//Variaveis de tempo e delay
unsigned long t_led,t_update_site,t_medicao;

//TODO remover essas globais
float t, adj;
//==========================================================================================================//
//										FUNCOES DE INTERRUPCAO
//==========================================================================================================//
void wspeedIRQ()
// Activated by the magnet in the anemometer (2 ticks per rotation), attached to input D3
{
  if (millis() - lastWindIRQ > 10) // Ignore switch-bounce glitches less than 10ms (142MPH max reading) after the reed switch closes
  {
    lastWindIRQ = millis(); //Grab the current time
    windClicks++; //There is 1.492MPH for each click per second.
  }
}

//==========================================================================================================//
//										SETUP INICIAL
//==========================================================================================================//
void setup()
{
	/*
	 * ========PERIFERICOS E SENSORES ============
	 */
	//wdt_reset();
	//wdt_enable(WDTO_8S);
	delay(2000); //Delay de startup
	Serial.begin(9600);
	//io_init();
	adc_init_10b();
	//lcd.init();                      // initialize the lcd


	/*
	 * ========INTERRUPCOES  ============
	 */
	  attachInterrupt(0, wspeedIRQ, FALLING);

	/*
	 * ========INICIALIZACOES============
	 */
	  wdt_reset();
	  //lcd.backlight();
	  //lcd.print(F("Display OK"));
	  //lcd.setCursor(0,1);
	  //lcd.print(F("Display OK 2"));
	  estado = INICIALIZANDO_INT_REDE;
	  //bmp085.init(MODE_ULTRA_HIGHRES, p0, false);
	  interrupts(); //sei();
	  Serial.println("Boot");

}

//#define debug
#define debug_serial

void loop()
{


#ifndef TESTES
	switch(estado)
	{
		case INICIALIZANDO_INT_REDE:
#ifdef debug
			lcd.clear();
			lcd.print(F("INICIALIZ_WIFI"));
#endif

			if (Ethernet.begin(mac) == OK)
			{
				LED = 1;
				estado = CONECTADO;
#ifdef debug_serial
				Serial.println("Conectado no Ethernet Sheld");
			}
			else Serial.println("Falha no Eth Shield");
#else
			}
#endif

		break;

		case CONECTADO:
#ifdef debug
			lcd.clear();
			lcd.print(F("CONECTADO"));
			lcd.setCursor(0,1);
			lcd.print(pressao);
			lcd.print(' ');
			lcd.print(temperatura/10.0,2);
#endif
			wdt_reset();

			if (millis() >= (T_UPDATE_VARIAVEIS+t_medicao))
			{
				t_medicao = millis();
				//bmp085.getPressure(&pressao);
//				pressao += 400;
				//bmp085.getTemperature(&temperatura);
//				temperatura += 10.0;
				vel_vento = 5.5;
				direcao_vento = 45;
			}
			if (millis() >= (T_UPDATE_SITE+t_update_site))
			{
				t_update_site = millis();
				conta_erros++;
				if (WunderWeather_posta_dados(vel_vento, direcao_vento,pressao,temperatura)==OK)
				{
					conta_erros = 0;
					estado_conectado = MEDINDO;
					wdt_reset();
					delay(5000);
#ifdef debug_serial
					Serial.println("Dados postados online");
#endif
				}
				else if (conta_erros>5) estado = INICIALIZANDO_INT_REDE;
			}

		break; //Fim do estado CONECTADO


		default:
			break;

	} //Fim do switch/case
	delay(50);
	if (millis() >= (T_PISCA_LED+t_led))
	{
		t_led = millis();
		if (estado == CONECTADO) t_led+= 300;
		LED = !LED;
	}
#endif
}//Fim do loop()


float get_wind_speed()
{
  float deltaTime = millis() - lastWindCheck; //750ms

  deltaTime /= 1000.0; //Covert to seconds

  float windSpeed = (float)windClicks / deltaTime; //3 / 0.750s = 4

  windClicks = 0; //Reset and start watching for new wind
  lastWindCheck = millis();


  windSpeed *= adj; //colocar aqui o fator de conversao.

  /* Serial.println();
   Serial.print("Windspeed:");
   Serial.println(windSpeed);*/

  return(windSpeed);
}
int16_t le_angulo_biruta()
{
	uint8_t index=0;
	if (SENSOR_DIR_1) index+=1;
	if (SENSOR_DIR_2) index+=2;
	if (SENSOR_DIR_3) index+=4;
	if (SENSOR_DIR_4) index+=8;
	if (SENSOR_DIR_5) index+=16;

	return angulos[index];
}

uint8_t WunderWeather_posta_dados(float vel_vento, uint16_t direcao_vento,float pressao,float temperatura)
{
	//Conecta no site
//	IPAddress ipx(192, 168, 0, 7);
	client.clearWriteError();
	client.stop();
	if (client.connect("weatherstation.wunderground.com", 80)== OK)
	//if (client.connect("rtupdate.wunderground.com", 80)== OK)
	//if( client.connect(ipx, 80) == OK)
	{
#ifdef debug_serial
	Serial.println("Conexao Site ok");
#endif
	}
	else
	{
#ifdef debug_serial
	Serial.println("Conexao Site ERRO");
#endif
	return 0;
	}
	wdt_reset();
	delay(500);

	client.print(F("GET /weatherstation/updateweatherstation.php?ID=IESPRITO5&PASSWORD=andref&dateutc=now&winddir="));
	client.print(direcao_vento);

	client.print(F("&windspeedmph="));
	client.print(vel_vento/1.6,1); //Conversao para mph, apenas um digito de precisao

	client.print(F("&tempf="));
	// Conversao Fahrenheit para Celsius -> °F = °C × 1, 8 + 32, temperatura vem x10 do BMP
	client.print((temperatura*0.18 +32.0),2);

	client.print(F("&baromin="));
	client.print(pressao*0.0295300586467*0.01,3);

	client.print(F("&action=updateraw  HTTP/1.0\r\nHost: \r\nConnection: close\r\n\r\n"));
	//client.print(F("&action=updateraw&realtime=1&rtfreq=2.5  HTTP/1.0\r\nHost: \r\nConnection: close\r\n\r\n"));


	delay(1000);

	if (!client.find("success"))
	{
#ifdef debug_serial
		Serial.println("Post no site Falhou");
#endif
		return 0;
	}
//	while (client.available())
//	{
//		    char c = client.read();
//		    Serial.print(c);
//	}
	return OK;
}
