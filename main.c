#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_rcc.h>
#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include <misc.h>
#include "rtty.h"
#include "init.h"
#include "config.h"
#include "radio.h"
#include "ublox.h"
#include "delay.h"
#include "aprs.h"
#include "dac.h"



void _send_uart3_char(unsigned char c) {
   USART_SendData(USART3, c);
   while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

void debug(char* str){
	while(*str) { _send_uart3_char(*str); str++; }
}


unsigned char TX_POWER=DEFAULT_TX_POWER;

/*
//-------FM TONE GENERATOR-------------------------------------------
unsigned char fm_sine_16[16]= {16,22,27,31,32,31,27,22,16,10,5,1,0,1,5,10 };
unsigned char fm_sine_4[4]= {16,32,16,0 };

volatile unsigned long sending_fm_tone;
void send_fm_tone_timer_16(){
	radio_rw_register(0x73, fm_sine_16[sending_fm_tone&0x0F], 1);
	if ((sending_fm_tone>0)==0) stop_timer(); else sending_fm_tone--;
}
void send_fm_tone_timer_4(){
	radio_rw_register(0x73, fm_sine_4[sending_fm_tone&0x03], 1);
	if ((sending_fm_tone>0)==0) stop_timer(); else sending_fm_tone--;
}


void send_fm_tone(unsigned long tone_hz, unsigned long duration_us, int blocking){
	if (tone_hz<800){
		sending_fm_tone=(sizeof(fm_sine_16)*tone_hz*duration_us)/1000000;
		init_timer_callback(1000000/(tone_hz*sizeof(fm_sine_16)),send_fm_tone_timer_16);
	}else {
		sending_fm_tone=(sizeof(fm_sine_4)*tone_hz*duration_us)/1000000;
		init_timer_callback(1000000/(tone_hz*sizeof(fm_sine_4)),send_fm_tone_timer_4);

	}
	if (blocking) while(sending_fm_tone>0);
}
*/



//-----------RTTY----
	char rtty_frame[500];

	rttyStates send_rtty_status = rttyZero;
volatile char *rtty_p;

	void rtty_timer(){
		 send_rtty_status = send_rtty(rtty_p);
		 if (send_rtty_status == rttyOne) {
			 radio_rw_register(0x73, 2, 1);
			 return;
		 }
		 if (send_rtty_status == rttyZero) {
			 radio_rw_register(0x73, 0, 1);
			 return;
		 }

		 if (send_rtty_status == rttyEnd) {
		       if (!(*rtty_p)){
		    	   stop_timer();
		    	   radio_disable_tx();
		    	}else rtty_p++;
		       return;
		 }


	}

//---APRS ----------------

	uint8_t afsk_phase;
	void AFSK_tone_timer(){
		//radio_rw_register(0x73, ((afsk_phase++)&0x01)?30:0, 1);//SPI deviation setting - MUCH slower, but also works
		((afsk_phase++)&0x01)?GPIO_SetBits(GPIOB, radioSDIpin):GPIO_ResetBits(GPIOB, radioSDIpin);
	}

	void AFSK_DAC_timer(){
		return;
	}



void main(void) {
 uint32_t i;
 ax25_t packet;
 uint8_t ctone;

  RCC_Conf();
  NVIC_Conf();
  init_port();

  delay_init();
  ublox_init();

  RED_LED_OFF;
  GREEN_LED_OFF;

  debug("\rstarted\n");

  radio_soft_reset();
  radio_rw_register(0x6D, 00 | (TX_POWER & 0x0007), 1);


  radio_rw_register(0x71, 0x00, 1); //unmodulated carrier
  radio_rw_register(0x73, 0, 1); //frequency offset


  //radio_rw_register(0x13, 0xF0, 1); // Temperature Value Offset
  //radio_rw_register(0x12, 0x00, 1); // Temperature Sensor Calibration
  //radio_rw_register(0x0f, 0x80, 1); // ADC configuration
  radio_set_tx_frequency(432.500f);




  while (1) {
	  //send_fm_tone(1195,10000,1);


#ifdef RTTY_BEACON
	  	RED_LED_ON;
	  	radio_set_tx_frequency(RTTY_BEACON);
		prepare_rtty_frame(rtty_frame);
		rtty_p=rtty_frame;
		//debug(rtty_frame);
		radio_enable_tx();
		_delay_ms(1000);
		init_timer_callback(6000000/RTTY_BAUDRATE,rtty_timer);
		while (*rtty_p) _delay_ms(1000);
		RED_LED_OFF;
#endif
		_delay_ms(2000); //let GPS push some new data...

#ifdef APRS_BEACON

		RED_LED_ON;
	  	radio_set_tx_frequency(APRS_BEACON);
		packet.data = rtty_frame;	packet.max_size = sizeof(rtty_frame); packet.mod = PROT_APRS_AFSK;
		ax25_init(&packet);
		aprs_encode_position(&packet);
		USART_Cmd(USART1, DISABLE); // turn off GPS data input (interrupts) to avoid transmission errors
		radio_enable_tx();
		radio_enable_direct_mode();
		_delay_ms(10);
		init_timer_callback(2500,AFSK_tone_timer);

/*
		///10s calibration loop
		for(i=0; i<12000; i++) {
			change_timer_period2((i&0x01)?1362:1362);
			_delay_us(799,0);
		}
*/

		ctone = 0;
		for(i=0; i < packet.size; i++) {
			if(((packet.data[i >> 3] >> (i & 0x7)) & 0x1) == 0) ctone ^= 0x01;
				change_timer_period2((ctone)?2500:1362);
				_delay_us(799,0);
		}
		_delay_ms(10);
		stop_timer();
		radio_disable_direct_mode();
		radio_disable_tx();
		RED_LED_OFF;
		USART_Cmd(USART1, ENABLE);

#endif
		_delay_ms(2000); //let GPS push some new data...


#ifdef AUDIO_APRS_BEACON
		RED_LED_ON;
		packet.data = rtty_frame;	packet.max_size = sizeof(rtty_frame); packet.mod = PROT_APRS_AFSK;
		ax25_init(&packet);
		aprs_encode_position(&packet);
		USART_Cmd(USART1, DISABLE); // turn off GPS data input (interrupts) to avoid transmission errors

		init_DAC();
		init_timer_callback(8800,AFSK_DAC_timer);

/*		///10s calibration loop
		for(i=0; i<12000; i++) {//10s CW for calibration
			change_timer_period((i&0x01)?84:84);
			_delay_us(780,0);
		}
*/
		ctone = 0;
		for(i=0; i < packet.size; i++) {
			if (((packet.data[i >> 3] >> (i & 0x7)) & 0x1) == 0) ctone ^= 0x01;
			change_timer_period((ctone)? 156 : 84);
			_delay_us(780,0);
		}

		_delay_ms(10);
		stop_timer();
		stop_DAC();
		RED_LED_OFF;
		USART_Cmd(USART1, ENABLE);

#endif


#ifdef HAB_USE
		//no switch off, rare led status
		if (gps_status==3) {
			debug("\rGPS FIX\n");
			GREEN_LED_ON;
			_delay_ms(50);
			GREEN_LED_OFF;
			_delay_ms(1000);
		}else {
			debug("\rGPS ACQ\n");
			RED_LED_ON;
			_delay_ms(50);
			RED_LED_OFF;
			_delay_ms(5000);
		}
		_delay_ms(SEND_INTERVAL);
#endif

#ifndef HAB_USE
		ctone=SEND_INTERVAL/1000;
		for (i=0;i<ctone;i++){
			if (gps_status==3) { GREEN_LED_ON; _delay_ms(50); GREEN_LED_OFF; }
				else { RED_LED_ON; _delay_ms(50); RED_LED_OFF; }
			_delay_ms(950);

			snprintf(rtty_frame,sizeof(rtty_frame),"\r%d, %d\n",ADCVal[0],ADCVal[1]);
			debug(rtty_frame);


			if (BUTTON_PRESSED){
				_delay_ms(100);
				if (BUTTON_PRESSED){//check for single spike etc.
					RED_LED_ON; GREEN_LED_ON; //ready for power off
					while (BUTTON_PRESSED) ; //wait for button release
					_delay_ms(200);
					power_off();
				}
			}
		}
#endif
		//tests - do not use
		/*
		ublox_sleep();
		_delay_ms(60000);
		ublox_wakeup();
		_delay_ms(15000);
		 */

		//NVIC_SystemLPConfig(NVIC_LP_SEVONPEND, DISABLE);
		//__WFI();



  }

}


