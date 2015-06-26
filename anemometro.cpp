
/*
   Descricao: Projeto de um anemometro remoto.

   Usa: Interface Serial


   Sensor:	Sensor hall e imas


        Atualizacoes
        	(13/02/2014)-> Primeiros testes
        	(25/06/2015) -> Implementado a mecanica do sensor de velocidade e da biruta eletronica.


 */





#include "anemometro.h"

//==========================================================================================================//
//										PROTOTIPOS DE FUNCOES
//==========================================================================================================//

float get_wind_speed();
int16_t le_angulo_biruta();

//==========================================================================================================//
//										VARIAVEIS GLOBAIS
//==========================================================================================================//
extern HardwareSerial Serial;
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
t_estados_wifi estado;
t_estados_c estado_conectado = MEDINDO;

volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;
long lastWindCheck = 0;

float vel_vento;
uint16_t direcao_vento;
float pressao;
float temperatura;

uint8_t conta_erros;
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
	Serial.begin(9600);
	io_init();
	adc_init_10b();
	lcd.init();                      // initialize the lcd

	/*
	 * ========INTERRUPCOES  ============
	 */
	  attachInterrupt(0, wspeedIRQ, FALLING);

	/*
	 * ========INICIALIZACOES============
	 */
	  delay(2000); //Delay de startup
	  lcd.backlight();
	  lcd.print(F("Display OK"));
	  lcd.setCursor(0,1);
	  lcd.print(F("Display OK 2"));
	  estado = INICIALIZANDO_WIFI;
	  interrupts(); //sei();

}

#define debug


void loop()
{

	 /*
	  * Fim do loop
	  */
//	lcd.clear();
//	lcd.print(##estado);
//	lcd.print(windClicks);
//	lcd.print(" p/s - ADC ");
//	lcd.print(adc_10bits(1));
//
//	adj = map(adc_10bits(1),5,760,100, 1000)*0.01;
//	t = get_wind_speed();
//	lcd.setCursor(0,1);
//	lcd.print(t,4);
//	lcd.print(" - ");
//	lcd.print(adj,2);
//	delay(1000);
//	tone_2(600,500);

//	if (SENSOR_DIR_5) lcd.print('1'); else lcd.print('0');
//	if (SENSOR_DIR_4) lcd.print('1'); else lcd.print('0');
//	if (SENSOR_DIR_3) lcd.print('1'); else lcd.print('0');
//	if (SENSOR_DIR_2) lcd.print('1'); else lcd.print('0');
//	if (SENSOR_DIR_1) lcd.print('1'); else lcd.print('0');
//	lcd.print('-');
	switch(estado)
	{
		case INICIALIZANDO_WIFI:
#ifdef debug
			lcd.clear();
			lcd.print(F("INICIALIZ_WIFI"));
#endif
			PWR_WIFI = 1;
			if (ESP_online()==OK)
			{
				LED = 1;
				estado = CONECTANDO_NA_REDE;
			}

		break;

		case CONECTANDO_NA_REDE:
#ifdef debug
			lcd.clear();
			lcd.print(F("CONECT_REDE"));
#endif
			if (ESP_conecta_rede()==OK) estado = CONECTADO;
			break;

		case CONECTADO:
			lcd.clear();
			lcd.print(F("CONECTADO"));
			delay(100);

			switch (estado_conectado)
			{
				case MEDINDO:
					estado_conectado = TRANSMITINDO_DADOS;

					//Atualizacao fake de variaveis.
					vel_vento = 17.5;
					direcao_vento = 180;
					pressao = 1014.2;
					temperatura = 32.7;
				break;

				case TRANSMITINDO_DADOS:
					conta_erros++;
					if (ESP_posta_dados(vel_vento, direcao_vento, pressao, temperatura)==OK)
					{
						conta_erros = 0;
						estado_conectado = MEDINDO;
						delay(5000);
					}
					else if (conta_erros>3) estado = INICIALIZANDO_WIFI;

				break;

				default: break;
			}


		break;




		default:
			break;

	} //Fim do switch/case
	delay(50);
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

