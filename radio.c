//
// Created by SQ5RWU on 2016-12-24.
//

#include "radio.h"

uint8_t _spi_sendrecv(const uint16_t data_word) {
  GPIO_ResetBits(GPIOC, radioNSELpin);
  // wait for tx buffer
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI2, data_word);

  // wait for data in rx buffer
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  GPIO_SetBits(GPIOC, radioNSELpin);
  return (uint8_t) SPI_I2S_ReceiveData(SPI2);
}

inline uint8_t radio_rw_register(const uint8_t register_addr, uint8_t value, uint8_t write){
  return _spi_sendrecv(((write ? register_addr | WR : register_addr) << 8) | value);
}

void radio_set_tx_frequency(const float freq_in_mhz) {
  uint8_t hbsel = (uint8_t) ((freq_in_mhz * (30.0f / SI4032_CLOCK)) >= 480.0f ? 1 : 0);

  uint8_t fb = (uint8_t) ((((uint8_t)((freq_in_mhz * (30.0f / SI4032_CLOCK)) / 10) - 24) - (24 * hbsel)) / (1 + hbsel));
  uint8_t gen_div  =  3;  // constant - not possible to change!
  uint16_t fc = (uint16_t) (((freq_in_mhz / ((SI4032_CLOCK / gen_div) * (hbsel + 1))) - fb - 24) * 64000);
  radio_rw_register(0x75, (uint8_t) (0b01000000 | (fb & 0b11111) | ((hbsel & 0b1) << 5)), 1);
  radio_rw_register(0x76, (uint8_t) (((uint16_t)fc >> 8) & 0xff), 1);
  radio_rw_register(0x77, (uint8_t) ((uint16_t)fc & 0xff), 1);
}





void radio_disable_tx() {
  radio_rw_register(0x07, 0x40, 1);
}

void radio_soft_reset() {
  radio_rw_register(0x07, 0x80, 1);
}

void radio_enable_tx() {
  radio_rw_register(0x07, 0x48, 1);
}

void radio_enable_direct_mode(){
    radio_rw_register(0x72, 5, 1); //frequency deviation
	GPIO_SetBits(GPIOC, radioNSELpin);
	//Direct Mode using TX Data via SDI pin (only when nSEL is high)
	//radio_rw_register(0x71, 0b00010011, 1);//modulation control GFSK
	radio_rw_register(0x71, 0b00010010, 1);//modulation control FSK
	spi_deinit();
}
void radio_disable_direct_mode(){
	GPIO_ResetBits(GPIOC, radioNSELpin);
	spi_init();
	radio_rw_register(0x71, 0x00, 1);//unmodulated carrier
    radio_rw_register(0x72, 0, 1); //frequency deviation
}




int8_t radio_read_temperature() {
  uint8_t temp;
  temp = radio_rw_register(0x11, 0xff, 0); // read ADC
  int8_t temperatura = (int8_t) (-64 + (temp * 5 / 10) - 16);
  radio_rw_register(0x0f, 0x80, 1);
  return temperatura;
}
