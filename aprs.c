/* trackuino copyright (C) 2010  EA5HAV Javi
 *  https://github.com/DL7AD/pecanpico9
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "config.h"
#include "ax25.h"
#include "aprs.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ublox.h"
#include "init.h"

#define METER_TO_FEET(m) (((m)*26876) / 8192)

static uint16_t loss_of_gps_counter;
static uint16_t msg_id;

/*
 //compressed format
void aprs_encode_position(ax25_t* packet){

	char temp[22];
	GPSEntry gpsData;
	ublox_get_last_data(&gpsData);

	// Encode header
	ax25_send_header(packet, APRS_CALLSIGN, APRS_SSID, APRS_PATH, APRS_PREAMBLE);
	ax25_send_byte(packet, '/');

	snprintf(temp, sizeof(temp), "%02d%02d%02dh", gpsData.hours, gpsData.minutes, gpsData.seconds);
	ax25_send_string(packet, temp);

	// Latitude
	uint32_t y = 380926 * (90 - gpsData.lat_raw/10000000.0);
	uint32_t y3  = y   / 753571;
	uint32_t y3r = y   % 753571;
	uint32_t y2  = y3r / 8281;
	uint32_t y2r = y3r % 8281;
	uint32_t y1  = y2r / 91;
	uint32_t y1r = y2r % 91;

	// Longitude
	uint32_t x = 190463 * (180 + gpsData.lon_raw/10000000.0);
	uint32_t x3  = x   / 753571;
	uint32_t x3r = x   % 753571;
	uint32_t x2  = x3r / 8281;
	uint32_t x2r = x3r % 8281;
	uint32_t x1  = x2r / 91;
	uint32_t x1r = x2r % 91;

	// Altitude
	//uint32_t a = logf(METER_TO_FEET(gpsData.alt_raw)) / logf(1.002f);
	//uint32_t a1  = a / 91;
	//uint32_t a1r = a % 91;
	uint32_t a = 0;
	uint32_t a1  = a / 91;
	uint32_t a1r = a % 91;


	uint8_t gpsFix = 0;//trackPoint->gps_lock == GPS_LOCKED ? GSP_FIX_CURRENT : GSP_FIX_OLD;
	uint8_t src = NMEA_SRC_GGA;
	uint8_t origin = ORIGIN_PICO;

	temp[0]  = (APRS_SYMBOL >> 8) & 0xFF;
	temp[1]  = y3+33;
	temp[2]  = y2+33;
	temp[3]  = y1+33;
	temp[4]  = y1r+33;
	temp[5]  = x3+33;
	temp[6]  = x2+33;
	temp[7]  = x1+33;
	temp[8]  = x1r+33;
	temp[9]  = APRS_SYMBOL & 0xFF;
	temp[10] = a1+33;
	temp[11] = a1r+33;
	temp[12] = ((gpsFix << 5) | (src << 3) | origin) + 33;
	temp[13] = 0;

	ax25_send_string(packet, temp);

	// Comments
	ax25_send_string(packet, "SATS ");
	snprintf(temp, sizeof(temp), "%d", gpsData.sats_raw);
	ax25_send_string(packet, temp);
	ax25_send_byte(packet, '|');

		// Encode footer
	ax25_send_footer(packet);
}
*/

void calc_DMH(long x, int8_t* d, uint8_t* m, uint8_t* h,uint8_t* hh) {
  unsigned int hhx;
  *d = (int8_t) (x / 1e7);
  x-= (*d * 1e7);
  if (x<0) x=-x;
  x*=60;
  x/=1000;
  hhx=x%100; hhx*=100; hhx/=110; *hh=hhx;
  x/=100;
  *m=x/100;
  *h=x%100;
}


void aprs_encode_position(ax25_t* packet){

	char temp[100];
	int8_t d;
	uint8_t m,h,hh_lat,hh_lon;

	GPSEntry gpsData;
	ublox_get_last_data(&gpsData);

	// Encode header
	ax25_send_header(packet, APRS_CALLSIGN, APRS_SSID, APRS_PATH, APRS_PREAMBLE);
	ax25_send_byte(packet, '@');

	snprintf(temp, sizeof(temp), "%02d%02d%02dh", gpsData.hours, gpsData.minutes, gpsData.seconds);
	ax25_send_string(packet, temp);

	calc_DMH(gpsData.lat_raw,&d,&m,&h,&hh_lat);
	snprintf(temp, sizeof(temp), "%02d%02u.%02u%c%c", (d>0)?d:-d,m,h, (d > 0) ? 'N' : 'S',(APRS_SYMBOL >> 8) & 0xFF);
	ax25_send_string(packet, temp);

	calc_DMH(gpsData.lon_raw,&d,&m,&h,&hh_lon);
	snprintf(temp, sizeof(temp), "%03d%02u.%02u%c%c%03d/%03d", (d>0)?d:-d,m,h, (d > 0) ? 'E' : 'W',APRS_SYMBOL& 0xFF,gpsData.heading_raw/100000,gpsData.speed_raw/27.777);
	ax25_send_string(packet, temp);

#ifdef HAB_USE
	snprintf(temp, sizeof(temp), "/A=%06d",METER_TO_FEET(gpsData.alt_raw/1000));
	ax25_send_string(packet, temp);
#endif

	snprintf(temp, sizeof(temp), "!w%c%c!",hh_lat+33,hh_lon+33);
	ax25_send_string(packet, temp);

#ifdef HAB_USE
	snprintf(temp, sizeof(temp), "Clb=%d.%02d ",gpsData.vspeed_raw/100,abs(gpsData.vspeed_raw%100));
	ax25_send_string(packet, temp);
#endif

#ifdef APRS_VOLTAGE
	uint32_t voltage = ADCVal[0] * 600 / 4096;
	snprintf(temp, sizeof(temp), "V=%d.%02d ", voltage/100, voltage-voltage/100*100);
	ax25_send_string(packet, temp);
#endif


	// Comments
	snprintf(temp, sizeof(temp), "SV=%d FIX=%d", gpsData.sats_raw, gpsData.fix);
	ax25_send_string(packet, temp);

	ax25_send_string(packet, APRS_COMMENT);
	ax25_send_footer(packet);
}









/*
void aprs_encode_init(ax25_t* packet, uint8_t* buffer, uint16_t size, mod_t mod)
{
	packet->data = buffer;
	packet->max_size = size;
	packet->mod = mod;

	// Encode APRS header
	ax25_init(packet);
}
uint32_t aprs_encode_finalize(ax25_t* packet)
{
	scramble(packet);
	nrzi_encode(packet);
	return packet->size;
}
*/
/**
 * Transmit message packet
 */
void aprs_encode_message(ax25_t* packet, const aprs_conf_t *config, const char *receiver, const char *text)
{
	char temp[20];

	// Encode header
	ax25_send_header(packet, config->callsign, config->ssid, config->path, packet->size > 0 ? 0 : config->preamble);
	ax25_send_byte(packet, ':');

	snprintf(temp, sizeof(temp), "%-9s", receiver);
	ax25_send_string(packet, temp);

	ax25_send_byte(packet, ':');
	ax25_send_string(packet, text);
	ax25_send_byte(packet, '{');

	snprintf(temp, sizeof(temp), "%d", ++msg_id);
	ax25_send_string(packet, temp);

	// Encode footer
	ax25_send_footer(packet);
}


