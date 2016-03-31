/*
 * tela.cpp
 *
 *  Created on: 26 de nov de 2015
 *      Author: Andre
 */


#include <tela.h>

PCD8544 TelaLCD = PCD8544(44, 46, 42, 40, 48);
enum telas { TELA_ANGULO_VP, TELA_NOME_IP, TELA_ANGULO_IP, TELA_NOME_VP};
enum telas tela_atual;

void updateLCD(ST_dados_metereologicos *dadosm, float cpainel, float vpnl)
{

	TelaLCD.clear();
	//PARTE DO UPDATE DE VELOCIDADE.
	char Buffer_txt[4] = "  ";
	if ((tela_atual == TELA_NOME_IP) || (tela_atual == TELA_NOME_VP))
	{
		if ((*dadosm).direcao_vento<24) strcpy_P(Buffer_txt,PSTR("N"));
		if (((*dadosm).direcao_vento>=24) && ((*dadosm).direcao_vento<=36)) strcpy_P(Buffer_txt,PSTR("NNE"));
		if (((*dadosm).direcao_vento>=48) && ((*dadosm).direcao_vento<=60)) strcpy_P(Buffer_txt,PSTR("NE "));
		if (((*dadosm).direcao_vento>=72) && ((*dadosm).direcao_vento<=84)) strcpy_P(Buffer_txt,PSTR("ENE"));
		if (((*dadosm).direcao_vento>=96) && ((*dadosm).direcao_vento<=108)) strcpy_P(Buffer_txt,PSTR(" E "));
		if (((*dadosm).direcao_vento>=120) && ((*dadosm).direcao_vento<=132)) strcpy_P(Buffer_txt,PSTR("ESE"));
		if (((*dadosm).direcao_vento>=144) && ((*dadosm).direcao_vento<=156)) strcpy_P(Buffer_txt,PSTR("SE "));
		if ((*dadosm).direcao_vento==168)  strcpy_P(Buffer_txt,PSTR("SSE"));
		if (((*dadosm).direcao_vento>=180) && ((*dadosm).direcao_vento<=192)) strcpy_P(Buffer_txt,PSTR(" S "));
		if (((*dadosm).direcao_vento>=204) && ((*dadosm).direcao_vento<=216)) strcpy_P(Buffer_txt,PSTR("SSO"));
		if (((*dadosm).direcao_vento>=228) && ((*dadosm).direcao_vento<=240)) strcpy_P(Buffer_txt,PSTR("SO "));
		if (((*dadosm).direcao_vento>=252) && ((*dadosm).direcao_vento<=264)) strcpy_P(Buffer_txt,PSTR("OSO"));
		if (((*dadosm).direcao_vento>=276) && ((*dadosm).direcao_vento<=288)) strcpy_P(Buffer_txt,PSTR(" O "));
		if (((*dadosm).direcao_vento>=300) && ((*dadosm).direcao_vento<=312)) strcpy_P(Buffer_txt,PSTR("ONO"));
		if (((*dadosm).direcao_vento>=324) && ((*dadosm).direcao_vento<=336)) strcpy_P(Buffer_txt,PSTR("NO "));
		if ((*dadosm).direcao_vento==348)  strcpy_P(Buffer_txt,PSTR("NNO"));	//Imprimindo direcao do vento.
		TelaLCD.setCursor(0,1);
		TelaLCD.imprime_grande(Buffer_txt);
		TelaLCD.setCursor(64,5);
		TelaLCD.drawbitmap(64,5,kph,19,7,BLACK);
	}
	else
	{
		itoa((*dadosm).direcao_vento,Buffer_txt,10);
		TelaLCD.setCursor(0,1);
		TelaLCD.imprime_grande(Buffer_txt);
		TelaLCD.setCursor(64,5);
		TelaLCD.drawbitmap(64,5,kph,19,7,BLACK);
	}
	//Imprimindo Velocidade
	TelaLCD.drawline(34,14,39,1,BLACK);
	TelaLCD.setCursor(41,0);
	itoa(dadosm->vel_vento,Buffer_txt,10);
	TelaLCD.imprime_grande(Buffer_txt);

	//Imprimindo rajada
	TelaLCD.setCursor(0,17);
	TelaLCD.print("Rajada:");
	TelaLCD.print((*dadosm).rajada); //Colocar os dados
	TelaLCD.drawbitmap(55,17,kph,19,7,BLACK);

	//Imprimindo pressao
	TelaLCD.setCursor(0,25);
	TelaLCD.print("Pressao:");
	TelaLCD.print((*dadosm).pressao/100);
	TelaLCD.drawbitmap(73,26,milibar,10,5,BLACK);

	//Imprimindo dados solares
	TelaLCD.setCursor(0,41);
	if ((tela_atual == TELA_NOME_VP) || (tela_atual == TELA_ANGULO_VP))
	{
		//Imprimindo Tensao
		TelaLCD.print("Vp:");
		TelaLCD.print(vpnl,1);
	}
	else
	{
		//Imprimindo Corrente
		TelaLCD.print("Ip:");
		TelaLCD.print(cpainel,1);
	}

	//Imprimindo relogio
	TelaLCD.setCursor(55,41);
	TelaLCD.print(':');
	TelaLCD.setCursor(69,41);
	TelaLCD.print(':');
	TelaLCD.setCursor(45,41);
	if (rtc.now().hour()<10) TelaLCD.print('0');
	TelaLCD.print(rtc.now().hour());
	TelaLCD.setCursor(59,41);
	if (rtc.now().minute()<10) TelaLCD.print('0');
	TelaLCD.print(rtc.now().minute());
	TelaLCD.setCursor(73,41);
	if (rtc.now().second()<10) TelaLCD.print('0');
	TelaLCD.print(rtc.now().second());
	//Imprimindo umidade
//	TelaLCD.setCursor(0,25);
//	TelaLCD.print("Pressao:");
//	TelaLCD.print(pressao/100);
//	TelaLCD.drawbitmap(73,26,milibar,10,5,BLACK);

	//Maquininha de estados;
	switch(tela_atual)
	{
		case TELA_ANGULO_VP:
			tela_atual = TELA_NOME_IP;
		break;
		case TELA_NOME_IP:
			tela_atual = TELA_ANGULO_IP;
		break;
		case TELA_ANGULO_IP:
			tela_atual = TELA_NOME_VP;
		break;
		default:
			tela_atual = TELA_ANGULO_VP;
		break;
	}
}
