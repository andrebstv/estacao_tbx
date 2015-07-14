
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
	wdt_reset();
	wdt_enable(WDTO_8S);
	delay(2000); //Delay de startup
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
	  wdt_reset();
	  lcd.backlight();
	  lcd.print(F("Display OK"));
	  lcd.setCursor(0,1);
	  lcd.print(F("Display OK 2"));
	  estado = INICIALIZANDO_WIFI;
	  bmp085.init(MODE_ULTRA_HIGHRES, p0, false);
	  interrupts(); //sei();
	  LED = 1;
	  estado = INICIALIZANDO_WIFI;

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
//	estado = CONECTADO;
	wdt_reset();
	switch(estado)
	{
		case INICIALIZANDO_WIFI:
#ifdef debug
			lcd.clear();
			lcd.print(F("INICIALIZ_WIFI"));
#endif
			PWR_WIFI = 1;
			RESET_ESP = 0;
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
			if (ESP_conecta_rede()==OK)
			{
				estado = CONECTADO;
				conta_conex_wifi = 0;
			}
			else
			{
				conta_conex_wifi++;
				if (conta_conex_wifi >=5) estado = INICIALIZANDO_WIFI;
			}
			break;

		case CONECTADO:
			lcd.clear();
			lcd.print(F("CONECTADO"));
			lcd.setCursor(0,1);
			lcd.print(pressao);
			lcd.print(' ');
			lcd.print(temperatura/10.0,2);

			wdt_reset();

			if (millis() >= (T_UPDATE_VARIAVEIS+t_medicao))
			{
				t_medicao = millis();
				bmp085.getPressure(&pressao);
				pressao += 400;
				bmp085.getTemperature(&temperatura);
				temperatura += 10.0;
				vel_vento = 5.5;
				direcao_vento = 45;
			}
			if (millis() >= (T_UPDATE_SITE+t_update_site))
			{
				t_update_site = millis();
				conta_erros++;
				if (ESP_posta_dados(vel_vento, direcao_vento, pressao, temperatura)==OK)
				{
					conta_erros = 0;
					estado_conectado = MEDINDO;
					wdt_reset();
					delay(5000);
				}
				else if (conta_erros>3) estado = INICIALIZANDO_WIFI;
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

