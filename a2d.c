/* ===============================================
 * 			Biblioteca para realizar conversoes A/D
 =================================================*/

#include "a2d.h"

void adc_init_8b()
{
	ADMUX |= (1<<REFS0); //Referencia em AVCC.
	ADMUX |= (1<<ADLAR); //Ajustando para o byte mais significativo em ADRH
	ADCSRA |= (1<<ADEN); //Ativando o modulo.
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //F_Sample = FOSC/128.
	_delay_ms(10); //Para estabilizacao
}
void adc_init_10b()
{
	ADMUX |= (1<<REFS0); //Referencia em AVCC.
//	ADMUX |= (1<<ADLAR); //Ajustando para o byte mais significativo em ADRH
	ADCSRA |= (1<<ADEN); //Ativando o modulo.
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //F_Sample = FOSC/128.
	_delay_ms(10);//Estabilizando.
}

unsigned char adc_8bits(unsigned char canal) //Canal ainda nao tem efeito.
{
	ADMUX = ( ADMUX & 0xF0 ) + canal;
	ADCSRA |= (1<<ADSC); //Comecando a conversao.
	while (bit_is_set(ADCSRA,ADSC)); //Espera acabar
	return ADCH;
}
int adc_10bits(unsigned char canal)
{
	int temp;
	ADMUX = ( ADMUX & 0xF0 ) + canal;
	ADCSRA |= (1<<ADSC); //Comecando a conversao.
	while (bit_is_set(ADCSRA,ADSC)); //Espera acabar
	temp = ADCL;
	temp += ADCH*256;
	return temp;
}
uint16_t fast_adc_10bits(uint8_t canal)
{
	static uint16_t temp;
	static uint8_t flag = 0;
	if (flag)
	{
		flag = 0;
		temp = ADCL;
		temp += ADCH*256;
		return temp;
	}
	else
	{
		ADMUX = ( ADMUX & 0xF0 ) + canal;
		adc_start();
		flag = 1;
		return temp;
	}

}
#define adc_start() ADCSRA |= (1<<ADSC)

int adc_10bits_sleep(unsigned char canal)
{
	int temp;
	set_sleep_mode(SLEEP_MODE_ADC);
	ADMUX = ( ADMUX & 0xF0 ) + canal;
	adc_start();
	sleep_mode();
	temp = ADCL;
	temp += ADCH*256;
	return temp;
}
