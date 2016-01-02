
/*
   Descricao: Projeto de um anemometro remoto.

   Usa: Interface Serial
   	   Modulo ESP8266 para conexao WiFi


   Sensor:	Sensor hall e imas


        Atualizacoes
        	(13/02/2014)-> Primeiros testes
        	(25/06/2015) -> Implementado a mecanica do sensor de velocidade e da biruta eletronica.
        	13/07/2015 -> ESP8266 Atualizado p/ versao nova do firmware. Melhoras significativas no boot.

        	v1.0 - 22/10/2015
        		-> Mudanca de bootloader para permitir atualizacao remota (primeiros testes)
        		-> Já postando online com o Ethernet Shield.
        		-> Bootloader e webreset funcionando.
        	v1.3 - 22/12/2015
        		-> Deploy em Timbuix, adicionado ajuste de angulo de biruta



 */





#include "anemometro.h"

//==========================================================================================================//
//										PROTOTIPOS DE FUNCOES
//==========================================================================================================//

int16_t le_angulo_biruta();
void resposta_dados();
uint8_t WunderWeather_posta_dados(float vel_vento, int16_t direcao_vento,float pressao,float temperatura,float umidade,float vpainel,float corrente);
//==========================================================================================================//
//										VARIAVEIS GLOBAIS
//==========================================================================================================//
//extern HardwareSerial Serial;
//LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display



t_estados_wifi estado;
t_estados_c estado_conectado = MEDINDO;

#define debug_serial
volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;

float vel_vento;
float umidade;
float vbat,corrente_painel,vpainel;
int16_t direcao_vento;
long pressao = 101325;
float temperatura;
volatile uint8_t conta_erros,conta_erros_shield;
volatile bool reset_check = false;
uint8_t tensao_saida;

SOFT_WDT WDT_Task_POST(60*2);
SOFT_WDT WDT_Task_Reset_server(10*2);
SOFT_WDT WDT_Task_LCD(10*2);
SOFT_WDT WDT_Task_Anemometro(10*2);
SOFT_WDT WDT_Task_Power_Management(60*2);


BMP085   bmp085 = BMP085();
DHT dht(A7,DHT22);
RTC_DS1307 rtc;
File myFile;

EthernetClient client; //Cliente de conexao
EthernetReset reset(8080); //Servidor de reset externo.
//Enderecos IPs
IPAddress ip(10, 0, 0, 11);
IPAddress dns_google(10,0,0,1);
IPAddress gateway_x(10,0,0,1);
IPAddress subnet(255,255,255,0);
//MAC Address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Variaveis de tempo e delay
//TODO Colocar na API do FreeRTOS
unsigned long t_led,t_update_site,t_medicao;


//==========================================================================================================//
//										FUNCOES DE INTERRUPCAO
//==========================================================================================================//
void Task_power_management(void *arg)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	for(;;)
	{
		//vbat = adc_10bits(10)*(5.0/1024)*0.180328;
		corrente_painel = (analogRead(A9)*(5000.0/1024.0)-2500.0)*(1.0/110.0);
		vpainel = analogRead(A10)*(5.0/1024.0)*5.511;
		Serial.println();
		Serial.print("VPainel=");
		Serial.println(vpainel,1);


		//Fim da Task
		WDT_Task_Power_Management.reset();
		vTaskDelayUntil(&xLastWakeTime,5000);
	}
}
void Task_Anemometro(void* arg)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	for(;;)
	{
		direcao_vento = le_angulo_biruta();
		vel_vento = vel_vento*FATOR_DE_FILTRAGEM_VENTO + windClicks *FATOR_DE_ANEMOMETRO*(1.0-FATOR_DE_FILTRAGEM_VENTO);
		Serial.println();
		Serial.print("VelVento = ");
		Serial.print(vel_vento,1);
		Serial.print("/");
		Serial.println(direcao_vento);
		Serial.flush();
		windClicks = 0;

		//Fim da Task
		WDT_Task_Anemometro.reset();
		vTaskDelayUntil(&xLastWakeTime,3000);

	}
}
void Task_LCD(void *arg)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	for(;;)
	{
		  updateLCD(pressao,direcao_vento,vel_vento,corrente_painel,vpainel);
		  TelaLCD.display();
		  WDT_Task_LCD.reset();
		  vTaskDelayUntil(&xLastWakeTime,T_UPDATE_LCD);
	}

}
static void Task_LED(void* arg)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		WDT_Task_POST.increment();
		WDT_Task_LCD.increment();
		WDT_Task_Anemometro.increment();
		WDT_Task_Power_Management.increment();

		if (WDT_Task_POST.check() && WDT_Task_Anemometro.check() && WDT_Task_LCD.check() && WDT_Task_Power_Management.check())
		{
			//Tudo ok pisque o led e limpe o WDT
			LED = !LED;
//			LED = SENSOR_VEL;
			Serial.print(WDT_Task_POST.count);
			Serial.flush();
			wdt_reset();
		}
		else
		{
			//Algo travou, reset geral
			Serial.println(" WDT Reboot");
			Serial.flush();
			grava_dados_SD("Reboot",rtc,myFile);
			vTaskDelay(100);
			cli();
			wdt_enable(WDTO_1S);
			for(;;) WIZNET_RST = 0;
		}
		vTaskDelayUntil(&xLastWakeTime,500);
	}
}
//Parte Serial Jogada na IdleHook
extern "C" {
	void vApplicationIdleHook( void )
	{
			if (serialEventRun) serialEventRun();
	}

	volatile uint8_t ultimo_estado = 0;
	void vApplicationTickHook()  //Usada para ler o estado do pino e contar os cliques.
	{

		if (ultimo_estado && (!SENSOR_VEL)) //Falling edge
		{
			windClicks++;
		}
		ultimo_estado = SENSOR_VEL;
	}
}
static void Task_POST(void* arg)
{

	conta_erros_shield = 0;
	conta_erros = 0;
	for(;;)
	{

		// Check de Dados, servidor de reset e upload.
		if (reset_check) {
			if (reset.check() ==1)
			{
				resposta_dados();
			}
			if (reset._client) reset._client.stop();
		}


		if (conta_erros_shield < 15)
			{
				WDT_Task_POST.reset(); //Previne uma zica brava.
			}
	#ifndef TESTES

		switch(estado)
		{
			case INICIALIZANDO_INT_REDE:

//				if (Ethernet.begin(mac) == OK)
				Ethernet.begin(mac,ip,dns_google,gateway_x,subnet);
				{
					vTaskDelay(1000);
					reset.begin();
					reset_check = true;
					conta_erros_shield = 0;
					estado = CONECTADO;
	#ifdef debug_serial
					Serial.println("Conectado no Ethernet Sheld");
					grava_dados_SD("Ethernet Iniciado",rtc,myFile);
					vTaskDelay(1000);
				}
//				else
//				{
//					conta_erros_shield++;
//				}

				//else Serial.println("Falha no Eth Shield");
	#endif

			break;

			case CONECTADO:
				if (millis() >= (T_UPDATE_VARIAVEIS+t_medicao))
				{
					t_medicao = millis();
					bmp085.getPressure(&pressao);
	//				pressao += 400;
					int32_t temp_temp=0;
					bmp085.getTemperature(&temp_temp);
					temperatura = temp_temp;
					temperatura = temperatura/10.0;
					Serial.println();
					Serial.print("Temperatura =");
					Serial.println(temperatura,1);
	//				temperatura = 10.0;
//					umidade = dht.readHumidity();
					umidade = 60;
//					temperatura = dht.readTemperature();
					direcao_vento = le_angulo_biruta();
	#ifdef debug_serial
					Serial.println(umidade,2);
					Serial.println(temperatura,1);
					Serial.println(pressao);
					Serial.print("Vento =");
					Serial.print(vel_vento);Serial.print('/');
					Serial.println(direcao_vento);
	#endif
				}
				if (millis() >= (T_UPDATE_SITE+t_update_site))
				{
					Serial.println("Estado_Tupd");
					t_update_site = millis();
					conta_erros++;
					if (WunderWeather_posta_dados(vel_vento, direcao_vento,pressao,temperatura,umidade,vpainel,corrente_painel)==OK)
					{
						conta_erros = 0;
						estado_conectado = MEDINDO;
	#ifdef debug_serial
						Serial.println("Dados postados online ");
						grava_dados_SD("Dados postados Online",rtc,myFile);
	#endif
					}
					else if (conta_erros >5 )
						{
							estado = INICIALIZANDO_INT_REDE;
//							conta_erros = 0;
							grava_dados_SD("Reset int rede",rtc,myFile);
						}
				}

			break; //Fim do estado CONECTADO


			default:
				break;

		} //Fim do switch/case
		vTaskDelay(200);

	#endif

	}//Fim do for
}//Fim da funcao


//==========================================================================================================//
//										SETUP INICIAL
//==========================================================================================================//
void setup()
{
	/*
	 * ========PERIFERICOS E SENSORES ============
	 */
	wdt_reset();
	wdt_disable();
	wdt_reset();
	wdt_enable(WDTO_8S);

	wdt_reset();
	//delay(2000); //Delay de startup
	Serial.begin(115200);
	io_init();
	WIZNET_RST = 1;
	adc_init_10b();
	//lcd.init();                      // initialize the lcd


	/*
	 * ========INTERRUPCOES  ============
	 */
	  //attachInterrupt(0, wspeedIRQ, FALLING);

	/*
	 * ========INICIALIZACOES============
	 */
	  wdt_reset();
	  estado = INICIALIZANDO_INT_REDE;
	  interrupts(); //sei();
	  dht.begin();
	  bmp085.init();
	  rtc.begin();

	  portBASE_TYPE s1, s2,s3,s4,s5;

	   pinMode(10, OUTPUT); // Necessário p/ a interface SPI funcionar.
	  if (!SD.begin(CS_SDCARD)) {
	    /*
	     *TODO Incluir aqui rotina de falha de inicializacao do SD Card
	     */
	  }
	  char logname[8];
	  sprintf(logname,"L%u%u%u.txt",rtc.now().day(),rtc.now().month(),rtc.now().year()%100);
	  myFile = SD.open(logname, FILE_WRITE);
	  grava_dados_SD("POWER ON",rtc,myFile);

	  // Task que vai piscar o LED
	  s1 = xTaskCreate(Task_LED, NULL, configMINIMAL_STACK_SIZE, NULL, TASK_LED_PRIORITY, NULL);
	  // Task que vai postar dados.
	  s2 = xTaskCreate(Task_POST, NULL, 500, NULL, TASK_POST_PRIORITY, NULL);
	  s3 = xTaskCreate(Task_Anemometro, NULL, configMINIMAL_STACK_SIZE, NULL, TASK_ANEMOMETRO_PRIORITY, NULL);
	  s4 = xTaskCreate(Task_LCD, NULL, 500, NULL, TASK_LCD_PRIORITY, NULL);
	  s5 = xTaskCreate(Task_power_management, NULL, configMINIMAL_STACK_SIZE, NULL, TASK_PWR_MANAGEMENT_PRIORITY, NULL);
	  //	  s2 = s3;


	  // check for creation errors
	  if (s1 != pdPASS || s2 != pdPASS || s3!= pdPASS || s4!= pdPASS || s5!= pdPASS) {
	    Serial.println(F("Creation problem"));
	    while(1);
	  }
	  // start scheduler
//	  delay(1000);
	  Serial.println(F("OS Pronto para Despache"));

	  TelaLCD.init(55);

	  vTaskStartScheduler();
	  Serial.println(F("Insufficient RAM"));
}

//#define debug


void loop()
{

}//Fim do loop()


int16_t le_angulo_biruta()
{
#define ANGULO_DE_AJUSTE 186
	int16_t angulo_temp=0;
	uint8_t index=0;
	if (SENSOR_DIR_1) index+=1;
	if (SENSOR_DIR_2) index+=2;
	if (SENSOR_DIR_3) index+=4;
	if (SENSOR_DIR_4) index+=8;
	if (SENSOR_DIR_5) index+=16;

	angulo_temp = angulos[index];

	if (angulo_temp>0)
	{
		if ((angulo_temp + ANGULO_DE_AJUSTE) > 360)
		{
			angulo_temp = angulo_temp - 360 + ANGULO_DE_AJUSTE;
		}
		else
		{
			angulo_temp += ANGULO_DE_AJUSTE;
		}
	}
	return angulo_temp;
//	return angulos[index];
}


uint8_t WunderWeather_posta_dados(float vel_vento, int16_t direcao_vento,float pressao,float temperatura,float umidade,float vpainel,float corrente)
{
	//Conecta no site
//	IPAddress ipx(192, 168, 0, 7);
	static bool realtime_turn = false;
	uint8_t resultado_da_conexao;
	client.clearWriteError();
	client.stop();
	char site[40];
	if (realtime_turn)
		resultado_da_conexao = client.connect("rtupdate.wunderground.com",80);
	else
		resultado_da_conexao = client.connect("weatherstation.wunderground.com", 80);

	if (resultado_da_conexao == OK)
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
	vTaskDelay(500);

	//Direcao do Vento
	client.print(F("GET /weatherstation/updateweatherstation.php?ID=IESPRITO5&PASSWORD=andref&dateutc=now&winddir="));
	client.print(direcao_vento);

	//Velocidade do Vento
	client.print(F("&windspeedmph="));
	client.print(vel_vento/1.6,1); //Conversao para mph, apenas um digito de precisao

	//Temperatura da estacao
	client.print(F("&tempf="));
	// Conversao Fahrenheit para Celsius -> °F = °C × 1, 8 + 32, temperatura vem x10 do BMP
	client.print((temperatura*1.8 +32.0),2);

	//Pressao Barometrica
	client.print(F("&baromin="));
	client.print(pressao*0.0295300586467*0.01,3);

	//umidade
	client.print(F("&humidity="));
	client.print(umidade,2);

	//Tensao do Painel Solar
	client.print(F("&dewptf="));
	client.print((vpainel*1.8 +32.0),2);
	//Corrente do painel.
	client.print(F("&soiltempf2="));
	client.print((corrente*1.8 +32.0),2);


	if (realtime_turn)
	{
		client.print(F("&action=updateraw&realtime=1&rtfreq=2.5  HTTP/1.0\r\nHost: \r\nConnection: close\r\n\r\n"));
		#ifdef debug_serial
			Serial.println(F("RT"));
		#endif
	}
	else
	{
		client.print(F("&action=updateraw  HTTP/1.0\r\nHost: \r\nConnection: close\r\n\r\n"));
		#ifdef debug_serial
			Serial.println(F("CM"));
		#endif
	}

	vTaskDelay(1000);

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
	realtime_turn = !realtime_turn;
	return OK;
}
void resposta_dados()
{
	reset._client.println("HTTP/1.1 200 OK");
	reset._client.println("Content-Type: text/html");
	reset._client.println("Connnection: close");
	reset._client.println();
	reset._client.println("<!DOCTYPE HTML>");
	reset._client.println("<html>");
	reset._client.println("Dados:<br>Tensao do Painel:");
		reset._client.print(vpainel,2);
		reset._client.print("<br>Corrente:");
		reset._client.print(corrente_painel,2);
		reset._client.print("<br>Horario:");
		reset._client.print(rtc.now().hour());reset._client.print(":");
		reset._client.print(rtc.now().minute());reset._client.print(":");
		reset._client.print(rtc.now().second());
		reset._client.print("<br>Vento:");
		reset._client.print(vel_vento,1);
		reset._client.print(" / Direcao:");
		reset._client.print(direcao_vento);
	reset._client.println("<br><br></html>");
	reset._client.flush();
	reset._client.stop();
}
void resposta_log(EthernetClient &cliente)
{

}
void grava_dados_SD (char *palavra, RTC_DS1307 &rtc_log, File &Log)
{
		     Log.print(rtc_log.now().day(), DEC);
		     Log.print('/');
		     Log.print(rtc_log.now().month(), DEC);
		     Log.print('/');
		     Log.print(rtc_log.now().year(), DEC);
		     Log.print(' ');
		     Log.print(rtc_log.now().hour(), DEC);
		     Log.print(':');
		     Log.print(rtc_log.now().minute(), DEC);
		     Log.print(':');
		     Log.print(rtc_log.now().second(), DEC);
		     Log.print(',');
		     //Imprime os dados
		     Log.println(palavra);
		     //Grava no SDCARD
		     Log.flush();
}

