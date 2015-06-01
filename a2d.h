/* ===============================================
 * 			Biblioteca para realizar conversoes A/D
 =================================================*/
#ifndef A2D_H
#define A2D_H 1
#include <avr/sleep.h>
#include <util/delay.h>
#endif
void adc_init_8b();
void adc_init_10b();
unsigned char adc_8bits(unsigned char canal); //Canal ainda nao tem efeito.
int adc_10bits(unsigned char canal);
#define adc_start() ADCSRA |= (1<<ADSC)
int adc_10bits_sleep(unsigned char canal);
uint16_t fast_adc_10bits(uint8_t canal);
