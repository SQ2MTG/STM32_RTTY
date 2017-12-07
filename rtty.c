#include <stdlib.h>
#include <string.h>
#include "rtty.h"
#include "ublox.h"
#include "init.h"

uint16_t gps_CRC16_checksum(char *string) {
  uint16_t crc = 0xffff;
  char i;
  while (*(string) != 0) {
    crc = crc ^ (*(string++) << 8);
    for (i = 0; i < 8; i++) {
      if (crc & 0x8000)
        crc = (uint16_t) ((crc << 1) ^ 0x1021);
      else
        crc <<= 1;
    }
  }
  return crc;
}



uint8_t start_bits;
char * callsign=CALLSIGN;

unsigned int rtty_frame_counter=0;

void prepare_rtty_frame (char *buf_rtty) {
  unsigned int len;

  start_bits = RTTY_PRE_START_BITS;
  int8_t si4032_temperature = 0;//radio_read_temperature();

  uint32_t voltage = ADCVal[0] * 600 / 4096;
  unsigned char flaga=0;
  unsigned int CRC_rtty=0;

  GPSEntry gpsData;
  ublox_get_last_data(&gpsData);
  if (gpsData.fix >= 3)  flaga |= 0x80;

  uint8_t lat_d = (uint8_t) abs(gpsData.lat_raw / 10000000);
  uint32_t lat_fl = (uint32_t) abs(abs(gpsData.lat_raw) - lat_d * 10000000) / 100;
  uint8_t lon_d = (uint8_t) abs(gpsData.lon_raw / 10000000);
  uint32_t lon_fl = (uint32_t) abs(abs(gpsData.lon_raw) - lon_d * 10000000) / 100;

  //debug(itoa(pkt->data.navposllh.lat,dbuf,10));

  sprintf(buf_rtty, "$$$$%s,%d,%02u%02u%02u,%s%d.%05ld,%s%d.%05ld,%ld,%d,%d.%02d,%d", callsign, rtty_frame_counter++,
                gpsData.hours, gpsData.minutes, gpsData.seconds,
                gpsData.lat_raw < 0 ? "-" : "", lat_d, lat_fl,
                gpsData.lon_raw < 0 ? "-" : "", lon_d, lon_fl,
                (gpsData.alt_raw / 1000), si4032_temperature, voltage/100, voltage-voltage/100*100, gpsData.sats_raw);


  /*
  sprintf(buf_rtty, "$$$$%s,%d,%02u%02u%02u,%s%d.%05ld,%s%d.%05ld,%ld,%d,%d.%d,%d,%d,%d,%02x", callsign, rtty_frame_counter,
              gpsData.hours, gpsData.minutes, gpsData.seconds,
              gpsData.lat_raw < 0 ? "-" : "", lat_d, lat_fl,
              gpsData.lon_raw < 0 ? "-" : "", lon_d, lon_fl,
              (gpsData.alt_raw / 1000), si4032_temperature, voltage/100, voltage-voltage/100*100, gpsData.sats_raw,
              gpsData.ok_packets, gpsData.bad_packets,
              flaga);
*/
  	  //len=strlen(buf_rtty);

  	  CRC_rtty = gps_CRC16_checksum(buf_rtty + 4);
  	  sprintf(buf_rtty, "%s*%04X\n", buf_rtty, CRC_rtty & 0xffff);

}


rttyStates send_rtty(char *buffer) {
  static uint8_t nr_bit = 0;
  nr_bit++;
  if (start_bits){
    start_bits--;
    return rttyOne;
  }

  if (nr_bit == 1) {
    return rttyZero;
  }
  if (nr_bit > 1 && nr_bit < (RTTY_7BIT ? 9 : 10)) {
    if ((*(buffer) >> (nr_bit - 2)) & 0x01) {
      return rttyOne;
    } else {
      return rttyZero;
    }
  }

#ifdef RTTY_7BIT
  nr_bit++;
#endif

  if (nr_bit == 10) {
    return rttyOne;
  }
#ifdef RTTY_USE_2_STOP_BITS
  if (nr_bit == 11) {
    return rttyOne;
  }
#endif

  nr_bit = 0;
  return rttyEnd;
}
