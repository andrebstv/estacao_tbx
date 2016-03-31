
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
        	v1.4 - 23/01/2015
        		-> Bateria nova, updates de rajadas e velocidade do vento filtrada.
        	v2.0 - 01/02/2015
        		-> Diversos updates, modularização do codigo.
        			-> Sistema on_off camera
        	v2.1 - 13/02/2015
        		-> Bugfixes... Procurando algo que possa estar travando a estacao.
        		-> Liga e desliga da camera automatico.

        	v2.2 - 27/02/2015
        		-> Modificado bootloader para proteger contra travamentos (add WDT )

 */





#include "anemometro.h"

//==========================================================================================================//
//										PROTOTIPOS DE FUNCOES
//==========================================================================================================//

int16_t le_angulo_biruta();
void resposta_dados();
bool updateThingSpeak();
long averagingFilter(long input);
float retorna_max_vento(float *array, uint8_t n);

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

float vbat;
float array_de_rajadas[(TEMPO_MEDICAO_RAJADA/TASK_ANEMOMETRO_PERIOD)];
volatile uint8_t conta_erros,conta_inicializacoes_de_rede;
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
AuxServer aux_server(8080);
//EthernetReset reset(8080); //Servidor de reset externo.
//Enderecos IPs
IPAddress ip_reporte(192, 168, 10, 8);
IPAddress ip_upload_firmware(192, 168, 10, 9);
IPAddress dns_google(200,175,5,139);
IPAddress gateway_x(192,168,10,1);
IPAddress subnet(255,255,255,0);
//MAC Address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte  mac_update[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};

//Variaveis de tempo e delay
//TODO Colocar na API do FreeRTOS
unsigned long t_led,t_update_site,t_medicao,t_update_ts;


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

//		dados_solares.corrente_painel = (averagingFilter(analogRead(A9))*(5000.0/1024.0)-2500.0)*(1.0/110.0);
		dados_solares.corrente_painel = (analogRead(A9)*(5000.0/1024.0)-2500.0)*(1.0/110.0);
		dados_solares.tensao_painel = averagingFilter(analogRead(A10))*(5.0/1024.0)*5.511;

		if (aux_server.modo_cam == CAM_AUTO)
		{
			if((rtc.now().hour()>=6) && (rtc.now().hour()<18))
			{
				aux_server.liga_cam();
			}
			else
			{
				aux_server.desliga_cam();
			}
		}

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
		dados_metereologicos.direcao_vento = le_angulo_biruta();
		dados_metereologicos.vel_vento = dados_metereologicos.vel_vento*FATOR_DE_FILTRAGEM_VENTO + windClicks *FATOR_DE_ANEMOMETRO*(1.0-FATOR_DE_FILTRAGEM_VENTO);
		Serial.println();
		Serial.print("VelVento = ");
		Serial.print(dados_metereologicos.vel_vento,1);
		Serial.print("/");
		Serial.println(dados_metereologicos.direcao_vento);
		Serial.flush();

		for (uint8_t i=1; i< (TEMPO_MEDICAO_RAJADA/TASK_ANEMOMETRO_PERIOD);i++)
		{
			array_de_rajadas[i-1] = array_de_rajadas[i];
		}
		array_de_rajadas[((TEMPO_MEDICAO_RAJADA/TASK_ANEMOMETRO_PERIOD)-1)] = windClicks *FATOR_DE_ANEMOMETRO;
		dados_metereologicos.rajada = retorna_max_vento(array_de_rajadas,(TEMPO_MEDICAO_RAJADA/TASK_ANEMOMETRO_PERIOD));

		windClicks = 0; //Zera para a proxima contagem.
//		rajada = retorna_max_vento(array_de_rajadas,3);
		//Fim da Task
		WDT_Task_Anemometro.reset();
		vTaskDelayUntil(&xLastWakeTime,TASK_ANEMOMETRO_PERIOD);

	}
}
void Task_LCD(void *arg)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	for(;;)
	{
		  updateLCD(&dados_metereologicos,dados_solares.corrente_painel,dados_solares.tensao_painel);
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
			wdt_disable();
			cli();
			wdt_enable(WDTO_2S);
			while(1);
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

	conta_inicializacoes_de_rede = 0;
	conta_erros = 0;
	for(;;)
	{
		/*
		 *  SERVIDOR AUXILIDAR DE DADOS, FORNECE:
		 *  		- INTERFACE DE RESET
		 *  		- INTERFACE DE DADOS
		 *  		- OFF e ON da CAMERA
		 *  		- SET DE HORA E IP
		 */
		if (reset_check)  //Se estiver habilitado.
		{
			aux_server.check(&dados_metereologicos,&dados_solares);
		}


		if (conta_inicializacoes_de_rede <= 10)
			{
				WDT_Task_POST.reset(); //Previne uma zica brava.
				_NOP();
			}
	#ifndef TESTES

		switch(estado)
		{
			case INICIALIZANDO_INT_REDE:

//				if (Ethernet.begin(mac) == OK)
				Ethernet.begin(mac,ip_reporte,dns_google,gateway_x,subnet);
				{
					vTaskDelay(1000);
					aux_server.begin();
					reset_check = true;
//					conta_inicializacoes_de_rede = 0;
					estado = CONECTADO;
	#ifdef debug_serial
					Serial.println("Conectado no Ethernet Sheld");
					grava_dados_SD("Ethernet Iniciado",rtc,myFile);
					vTaskDelay(1000);
				}
//				else
//				{
					conta_inicializacoes_de_rede++;
//				}

				//else Serial.println("Falha no Eth Shield");
	#endif

			break;

			case CONECTADO:
				if (millis() >= (T_UPDATE_VARIAVEIS+t_medicao))
				{
					t_medicao = millis();
					bmp085.getPressure(&dados_metereologicos.pressao);
					int32_t temp_temp=0;
					bmp085.getTemperature(&temp_temp);
					dados_metereologicos.temperatura = temp_temp;
					dados_metereologicos.temperatura = dados_metereologicos.temperatura/10.0;
	//				temperatura = 10.0;
//					umidade = dht.readHumidity();
					dados_metereologicos.umidade = 60;
//					temperatura = dht.readTemperature();
				}
				if (millis() >= (T_UPDATE_SITE+t_update_site))
				{
					Serial.println("Estado_Tupd");
					t_update_site = millis();
					conta_erros++;
					if (WunderWeather_posta_dados(&dados_metereologicos,dados_solares.tensao_painel,dados_solares.corrente_painel)==OK)
					{
						conta_erros = 0;
						conta_inicializacoes_de_rede = 0;
						estado_conectado = MEDINDO;
	#ifdef debug_serial
						Serial.println("Dados postados online ");
						grava_dados_SD("Dados postados Online",rtc,myFile);
	#endif
					}
					else if (conta_erros >5 )
						{
							estado = INICIALIZANDO_INT_REDE;
							conta_erros = 0;
							grava_dados_SD("Reset int rede",rtc,myFile);
						}
				}
				if (millis() >= (30000+t_update_ts))
				{
					if (updateThingSpeak()) conta_erros = 0;
					t_update_ts = millis();
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
	LED = 1;
	WIZNET_RST = 1;
	CAM_PWR1 = 1;
	CAM_PWR2 = 1;
	adc_init_10b();


	/*
	 * ========INTERRUPCOES  ============
	 */
	  //attachInterrupt(0, wspeedIRQ, FALLING);

	/*
	 * ========INICIALIZACOES============
	 */
	  wdt_reset();
	  /*
	   * TRECHO DE CODIGO QUE MUDA O IP.
	   * NAO MANTER.
	   */
	  word port = 8081;
	  NetEEPROM.writeNet(mac_update, ip_upload_firmware, gateway_x, subnet);
	  estado = INICIALIZANDO_INT_REDE;
	  interrupts(); //sei();

	  rtc.begin();
	  aux_server._rtc = &rtc; //Apontanto o RTC para a classe.

	   pinMode(10, OUTPUT); // Necessário p/ a interface SPI funcionar.
	  TelaLCD.init(55);
	  if (!SD.begin(CS_SDCARD)) {
	    /*
	     *TODO Incluir aqui rotina de falha de inicializacao do SD Card
	     */
	  }
	  char logname[8];
	  sprintf(logname,"L%u%u%u.txt",rtc.now().day(),rtc.now().month(),rtc.now().year()%100);
	  myFile = SD.open(logname, FILE_WRITE);
	  grava_dados_SD("POWER ON",rtc,myFile);

	  portBASE_TYPE s1, s2,s3,s4,s5;

	  dht.begin();
	  bmp085.init();
	  bmp085.setAltOffset(57000);

	  Serial.println(" ");
	  Serial.print("Init completo\r\n - Init do OS");
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


	  vTaskStartScheduler();
	  Serial.println(F("Insufficient RAM"));
}

//#define debug


void loop()
{

}//Fim do loop()


int16_t le_angulo_biruta()
{
#define ANGULO_DE_AJUSTE 228
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


uint8_t WunderWeather_posta_dados(ST_dados_metereologicos *dados_met,float vpainel,float corrente)
{
	//Conecta no site
//	IPAddress ipx(192, 168, 0, 7);
	static bool realtime_turn = false;
	uint8_t resultado_da_conexao;
	client.clearWriteError();
	client.stop();
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
	client.print(dados_met->direcao_vento);

	//Velocidade do Vento
	client.print(F("&windspeedmph="));
	client.print(dados_met->vel_vento/1.6,1); //Conversao para mph, apenas um digito de precisao

	//Rajada do Vento
	client.print(F("&windgustmph="));
	client.print(dados_met->rajada/1.6,1); //Conversao para mph, apenas um digito de precisao

	//Temperatura da estacao
	client.print(F("&tempf="));
	// Conversao Fahrenheit para Celsius -> °F = °C × 1, 8 + 32, temperatura vem x10 do BMP
	client.print((dados_met->temperatura*1.8 +32.0),2);

	//Pressao Barometrica
	client.print(F("&baromin="));
	client.print(dados_met->pressao*0.0295300586467*0.01,3);

	//umidade
	client.print(F("&humidity="));
	client.print(dados_met->umidade,2);

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
float retorna_max_vento(float *array, uint8_t n)
{
	float max_temp = -1000.0;
	for (uint8_t i = 0; i<n; i++)
	{
		if (*array > max_temp) max_temp = *array;
		array++;
	}
	return max_temp;
}
bool updateThingSpeak()
{
//	    String postStr;
//	           postStr +="&field1=";
//	           postStr += String(dados_solares.tensao_painel);
//	           postStr +="&field2=";
//	           postStr += String(dados_solares.corrente_painel);

	IPAddress ip_ts(184, 106, 153, 149);

	client.clearWriteError();
	client.stop();

	  if (client.connect(ip_ts, 80)) //api.thingspeak.com nao funciona... vai saber.
	  {
//	    client.print(F("POST /update HTTP/1.1\n"));
//	    client.print(F("Host: api.thingspeak.com\n"));
//	    client.print(F("Connection: close\n"));
//	    client.print(F("X-THINGSPEAKAPIKEY: 0VC2U7KAIGL5AWHS\n")); //API Key do Site
//	    client.print(F("Content-Type: application/x-www-form-urlencoded\n"));
//	    client.print(F("Content-Length:"));
//	    client.print(postStr.length());
//	    client.print(F("\n\n"));
//	    client.print(postStr);

	    client.print(F("GET /update?key=0VC2U7KAIGL5AWHS&field1="));
	    	    client.print(dados_solares.tensao_painel,2);
	    client.print(F("&field2="));
	    	    client.print(dados_solares.corrente_painel,2);
	    client.print(F("  HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\n\r\n"));
		//USAR O GET COMO É FEITO NO WUNDERGROUND É BEM MAIS FACIL
		//NAO ESQUECER DE USAR A STRING IGUAL ESTA ABAIXO
		//GET /update?key=0VC2U7KAIGL5AWHS&field1=-10.31&field2=-33.12  HTTP/1.1<CR><LF>Host: api.thingspeak.com<CR><LF>Connection: close<CR><LF><CR><LF>
	    	if (client.find("Status: 200 OK"))
	    	{
	    	    return true;
	    	}
	    	else return false;

	  }
	  return false;
}

#define SAMPLES_ARR 8
static long k[SAMPLES_ARR];
long averagingFilter(long input) // moving average filter function
{
	long sum = 0;
	for (int i = 0; i < SAMPLES_ARR; i++) {
		k[i] = k[i+1];
	}
	k[SAMPLES_ARR - 1] = input;
	for (int i = 0; i < SAMPLES_ARR; i++) {
		sum += k[i];
	}
	return ( sum / SAMPLES_ARR ) ;
}
