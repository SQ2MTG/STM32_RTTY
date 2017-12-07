/* trackuino copyright (C) 2010  EA5HAV Javi
 * https://github.com/DL7AD/pecanpico9
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

#ifndef __AX25_H__
#define __AX25_H__
#include <stm32f10x.h>

typedef struct {
	char callsign[7];
	unsigned char ssid;
} address_t;

typedef enum { // Modulation type
	MOD_NOT_SET,
	MOD_OOK,
	MOD_2FSK,
	MOD_2GFSK,
	MOD_AFSK
} mod_t;

// Protocol type
typedef enum {
	PROT_NOT_SET,
	PROT_SSDV_2FSK,
	PROT_APRS_AFSK,
	PROT_APRS_2GFSK,
	PROT_UKHAS_2FSK,
	PROT_MORSE
} prot_t;

typedef enum {
	CONF_PARM,
	CONF_UNIT,
	CONF_EQNS,
	CONF_BITS
} telemetry_conf_t;
/*
typedef enum {
	TEL_SATS,
	TEL_TTFF,
	TEL_VBAT,
	TEL_VSOL,
	TEL_PBAT,
	TEL_RBAT,
	TEL_PRESS,
	TEL_TEMP,
	TEL_HUM
} telemetry_t;

typedef struct {
	uint32_t id;			// Serial ID
//	ptime_t time;			// GPS time

	// GPS
	int32_t gps_lat;		// Latitude in °*10^7
	int32_t gps_lon;		// Longitude in °*10^7
	uint16_t gps_alt;		// Altitude in meter
	uint8_t gps_sats;		// Satellites used for solution
	uint8_t gps_ttff;		// Time to first fix in seconds

	// Voltage and current measurement
	uint16_t adc_vsol;		// Current solar voltage in mV
	uint16_t adc_vbat;		// Current battery voltage in mV
	uint16_t adc_vusb;		// Current USB voltage in mV
	int16_t adc_pbat;		// Average battery current (since last track point)
	int16_t adc_rbat;		// Battery impedance

	// BME280 (on board)
	uint32_t air_press;		// Airpressure in Pa*10 (in 0.1Pa)
	uint16_t air_hum;		// Rel. humidity in %*10 (in 0.1%)
	int16_t air_temp;		// Temperature in degC*100 (in 0.01°C)

	uint8_t id_image;		// Last image ID (this is important because it will set the image counter at reset so the last image wont get overwritten with the same image ID)
} trackPoint_t;
*/
typedef struct {
	char callsign[16];			// APRS callsign
	uint8_t ssid;				// APRS SSID
	uint16_t symbol;			// APRS symbol
	char path[16];				// APRS path
	uint16_t preamble;			// Preamble in milliseconds
//	telemetry_t tel[5];			// Telemetry types
	uint8_t tel_enc;				// Transmit telemetry encoding information
	uint16_t tel_enc_cycle;		// Telemetry encoding cycle in seconds
	char tel_comment[32];		// Telemetry comment
} aprs_conf_t;

typedef struct {
	uint8_t ones_in_a_row;	// Ones in a row (for bitstuffing)
	uint8_t *data;			// Data
	uint16_t size;			// Packet size in bits
	uint16_t max_size;		// Max. Packet size in bits (size of modem packet)
	uint16_t crc;			// CRC
	mod_t mod;				// Modulation type (MOD_AFSK or MOD_2GFSK)
} ax25_t;

void ax25_init(ax25_t *packet);
void ax25_send_header(ax25_t *packet, const char *callsign, uint8_t ssid, const char *path, uint16_t preamble);
void ax25_send_path(ax25_t *packet, const char *callsign, uint8_t ssid, uint8_t last);
void ax25_send_byte(ax25_t *packet, char byte);
void ax25_send_string(ax25_t *packet, const char *string);
void ax25_send_footer(ax25_t *packet);
void scramble(ax25_t *packet);
void nrzi_encode(ax25_t *packet);

#endif

