//
// Created by SQ5RWU on 2016-12-27.
//

#include <stm32f10x_usart.h>
#include <stm32f10x_gpio.h>
#include <string.h>
#include "ublox.h"
#include "delay.h"
#include "init.h"

volatile uint8_t gps_status=0;

char dbuf[20];

void USART1_IRQHandler(void) {
	unsigned char c;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == RESET) return;
  //if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	c=USART_ReceiveData(USART1);
	ublox_handle_incoming_byte(c);
    //sprintf(dbuf,"%02X ",c);
    //debug(dbuf);

  //USART_SendData(USART3, c);
  //while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}
/*
void USART1_IRQHandler(void) {
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
	  	  ublox_handle_incoming_byte((uint8_t) USART_ReceiveData(USART1));
  }else if (USART_GetITStatus(USART1, USART_IT_ORE) != RESET) { USART_ReceiveData(USART1); } else {    USART_ReceiveData(USART1);  }
}
*/




GPSEntry currentGPSData;
volatile uint8_t active = 0;
volatile uint8_t ack_received = 0;
volatile uint8_t nack_received = 0;

void _sendSerialByte(uint8_t message) {
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {
  }
  USART_SendData(USART1, message);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {
  }
}

void send_ublox(uint8_t msgClass, uint8_t msgId, uint8_t *payload, uint16_t payloadSize) {
  uBloxChecksum chksum = ublox_calc_checksum(msgClass, msgId, payload, payloadSize);

  _sendSerialByte(0xB5);
  _sendSerialByte(0x62);
  _sendSerialByte(msgClass);
  _sendSerialByte(msgId);
  _sendSerialByte((uint8_t) (payloadSize & 0xff));
  _sendSerialByte((uint8_t) (payloadSize >> 8));

  uint16_t i;
  for (i = 0; i < payloadSize; ++i) {
    _sendSerialByte(payload[i]);
  }
  _sendSerialByte(chksum.ck_a);
  _sendSerialByte(chksum.ck_b);
}

void send_ublox_packet(uBloxPacket * packet){
  send_ublox(packet->header.messageClass, packet->header.messageId, (uint8_t*)&packet->data, packet->header.payloadSize);
}

uBloxChecksum ublox_calc_checksum(const uint8_t msgClass, const uint8_t msgId, const uint8_t *message, uint16_t size) {
  uBloxChecksum ck = {0, 0};
  uint8_t i;
  ck.ck_a += msgClass;
  ck.ck_b += ck.ck_a;
  ck.ck_a += msgId;
  ck.ck_b += ck.ck_a;

  ck.ck_a += size & 0xff;
  ck.ck_b += ck.ck_a;
  ck.ck_a += size >> 8;
  ck.ck_b += ck.ck_a;


  for (i =0;i<size;i++){
    ck.ck_a += message[i];
    ck.ck_b += ck.ck_a;
  }
  return ck;
}

void ublox_get_last_data(GPSEntry * gpsEntry){
  __disable_irq();
  memcpy(gpsEntry, &currentGPSData, sizeof(GPSEntry));
  __enable_irq();
}

void ublox_enable_pm(){
	uBloxPacket msgcfgpm = {.header = {0xb5, 0x62, .messageClass=0x06, .messageId=0x32, .payloadSize=sizeof(uBloxCFGPM)},
	      .data.cfgpm = {.version=0, .flags=0, .updatePeriod=0, .searchPeriod=0, .gridOffset=0, .onTime=0, .minAcqTime=0 }};

	//uBloxPacket msgcfgpm = {.header = {0xb5, 0x62, .messageClass=0x06, .messageId=0x32, .payloadSize=sizeof(uBloxCFGPM)},
	//	      .data.cfgpm = {.version=0, .flags=0x18102, .updatePeriod=5000, .searchPeriod=60000, .gridOffset=0, .onTime=10, .minAcqTime=15 }};

	  do {
	    send_ublox_packet(&msgcfgpm);
	  } while (!ublox_wait_for_ack());

}

void ublox_sleep(){
	gps_status=0; //SLEEP
	uBloxPacket msgcfgpm = {.header = {0xb5, 0x62, .messageClass=0x02, .messageId=0x41, .payloadSize=8},
	      .data.rxmpmreq = {.duration=0, .flags=0x02 }};
	send_ublox_packet(&msgcfgpm);
}

void ublox_wakeup(){
	_sendSerialByte(0xB5);
}

void ublox_init(){

//RESET GPS

//COLDSTART
  //uBloxPacket msgcfgrst = {.header = {0xb5, 0x62, .messageClass=0x06, .messageId=0x04, .payloadSize=sizeof(uBloxCFGRSTPayload)},
  //    .data.cfgrst = { .navBbrMask=0xffff, .resetMode=1, .reserved1 = 0}
  //};

	//HOTSTART
	uBloxPacket msgcfgrst = {.header = {0xb5, 0x62, .messageClass=0x06, .messageId=0x04, .payloadSize=sizeof(uBloxCFGRSTPayload)},
	      .data.cfgrst = { .navBbrMask=0x0000, .resetMode=1, .reserved1 = 0}
	};


  init_usart_gps(38400, 1);
  _delay_ms(10);
  send_ublox_packet(&msgcfgrst);
  _delay_ms(800);
  init_usart_gps(9600, 1);
  _delay_ms(10);
  send_ublox_packet(&msgcfgrst);
  _delay_ms(800);

  uBloxPacket msgcgprt = {.header = {0xb5, 0x62, .messageClass=0x06, .messageId=0x00, .payloadSize=sizeof(uBloxCFGPRTPayload)},
      .data.cfgprt = {.portID=1, .reserved1=0, .txReady=0, .mode=0b00100011000000, .baudRate=38400,
          .inProtoMask=1, .outProtoMask=1, .flags=0, .reserved2={0,0}}};
  send_ublox_packet(&msgcgprt);
  init_usart_gps(38400, 1);

  _delay_ms(10);

  uBloxPacket msgcfgrxm = {.header = {0xb5, 0x62, .messageClass=0x06, .messageId=0x11, .payloadSize=sizeof(uBloxCFGRXMPayload)},
      .data.cfgrxm = {.reserved1=8, .lpMode=4}};

  do {
    send_ublox_packet(&msgcfgrxm);
  } while (!ublox_wait_for_ack());

  //Message Poll requests
  //06 01
  //06 06
  //06 21
  //06 12
  debug("\rCONFIGURE POLLING 06 01\n");
  uBloxPacket msgcfgmsg = {.header = {0xb5, 0x62, .messageClass=0x06, .messageId=0x01, .payloadSize=sizeof(uBloxCFGMSGPayload)},
    .data.cfgmsg = {.msgClass=0x01, .msgID=0x02, .rate=1}};

  do {
    send_ublox_packet(&msgcfgmsg);
  } while (!ublox_wait_for_ack());


  debug("\rCONFIGURE POLLING 06 06\n");
  msgcfgmsg.data.cfgmsg.msgID = 0x06;
  do {
    send_ublox_packet(&msgcfgmsg);
  } while (!ublox_wait_for_ack());

  debug("\rCONFIGURE POLLING 06 21\n");
  msgcfgmsg.data.cfgmsg.msgID = 0x21;
  do {
    send_ublox_packet(&msgcfgmsg);
  } while (!ublox_wait_for_ack());


    msgcfgmsg.data.cfgmsg.msgID = 0x12; //NAV-VELNED
    do {
      send_ublox_packet(&msgcfgmsg);
    } while (!ublox_wait_for_ack());

  //config NAV5
  //dynModel 7 = Airborne <2g
  //fixmode 3d only


  uBloxPacket msgcfgnav5 = {.header = {0xb5, 0x62, .messageClass=0x06, .messageId=0x24, .payloadSize=sizeof(uBloxCFGNAV5Payload)},
    .data.cfgnav5={.mask=0b00000001111111111, .dynModel=7, .fixMode=2, .fixedAlt=0, .fixedAltVar=10000, .minElev=5, .drLimit=0, .pDop=25, .tDop=25,
                   .pAcc=100, .tAcc=300, .staticHoldThresh=0, .dgpsTimeOut=2, .reserved2=0, .reserved3=0, .reserved4=0}};
  do {
    send_ublox_packet(&msgcfgnav5);
  } while (!ublox_wait_for_ack());
}

void ublox_handle_incoming_byte(uint8_t data){
  static uint8_t sync = 0;
  static uint8_t buffer_pos = 0;
  static uint8_t incoming_packet_buffer[sizeof(uBloxPacket) + sizeof(uBloxChecksum)];
  static uBloxPacket * incoming_packet = (uBloxPacket *) incoming_packet_buffer;
  if (!sync){
    if (!buffer_pos && data == 0xB5){
      buffer_pos = 1;
      incoming_packet->header.sc1 = data;
    } else if (buffer_pos == 1 && data == 0x62){
      sync = 1;
      buffer_pos = 2;
      incoming_packet->header.sc2 = data;
    } else {
      buffer_pos = 0;
    }
  } else {
    ((uint8_t *)incoming_packet)[buffer_pos] = data;
    if ((buffer_pos >= sizeof(uBloxHeader)-1) && (buffer_pos-1 == (incoming_packet->header.payloadSize + sizeof(uBloxHeader) + sizeof(uBloxChecksum)))){
      ublox_handle_packet((uBloxPacket *) incoming_packet);
      buffer_pos = 0;
      sync = 0;
    } else {
      buffer_pos++;
      if (buffer_pos >= sizeof(uBloxPacket) + sizeof(uBloxChecksum)) {
        buffer_pos = 0;
        sync = 0;
      }
    }
  }
}

void ublox_handle_packet(uBloxPacket *pkt) {
  uBloxChecksum cksum = ublox_calc_checksum(pkt->header.messageClass, pkt->header.messageId, (const uint8_t *) &pkt->data, pkt->header.payloadSize);
  uBloxChecksum *checksum = (uBloxChecksum *)(((uint8_t*)&pkt->data) + pkt->header.payloadSize);
  if (cksum.ck_a != checksum->ck_a || cksum.ck_b != checksum->ck_b) {
    currentGPSData.bad_packets += 1;
    return;
  }


/*
  	 if (pkt->header.messageClass == 0x01 && pkt->header.messageId == 0x07){ //NAV-PVT -- UNSUPPORTED IN THIS GPS
      currentGPSData.ok_packets += 1;
      currentGPSData.fix = pkt->data.navpvt.fixType;
      currentGPSData.lat_raw = pkt->data.navpvt.lat;
      currentGPSData.lon_raw = pkt->data.navpvt.lon;
      currentGPSData.alt_raw = pkt->data.navpvt.hMSL;
      currentGPSData.hours = pkt->data.navpvt.hour;
      currentGPSData.minutes = pkt->data.navpvt.min;
      currentGPSData.seconds = pkt->data.navpvt.sec;
      currentGPSData.sats_raw = pkt->data.navpvt.numSV;
      currentGPSData.speed_raw = pkt->data.navpvt.gSpeed;

    } else
*/
    if (pkt->header.messageClass == 0x01 && pkt->header.messageId == 0x02){ //NAV-POSLLH
      currentGPSData.ok_packets += 1;
      currentGPSData.lat_raw = pkt->data.navposllh.lat;
      currentGPSData.lon_raw = pkt->data.navposllh.lon;
      currentGPSData.alt_raw = pkt->data.navposllh.hMSL;
      return;
    }
    if (pkt->header.messageClass == 0x01 && pkt->header.messageId == 0x06){//NAV-SOL, TOW, ECEF, DOP etc.
      currentGPSData.sats_raw = pkt->data.navsol.numSV;
      currentGPSData.fix = pkt->data.navsol.gpsFix;
      /*0x00 = No Fix
    		  0x01 = Dead Reckoning only
    		  0x02 = 2D-Fix
    		  0x03 = 3D-Fix
    		  0x04 = GPS + dead reckoning combined
    		  0x05 = Time only fix
    		  0x06..0xff: reserved
     */
      if (pkt->data.navsol.gpsFix==3) gps_status=3; //FIX
      	  else gps_status=1; //ACQUARING
      return;
    }
    if (pkt->header.messageClass == 0x01 && pkt->header.messageId == 0x21){//NAV-TIMEUTC
      currentGPSData.hours = pkt->data.navtimeutc.hour;
      currentGPSData.minutes = pkt->data.navtimeutc.min;
      currentGPSData.seconds = pkt->data.navtimeutc.sec;
      return;
    }
    if (pkt->header.messageClass == 0x01 && pkt->header.messageId == 0x12){//NAV-VELNED
    	currentGPSData.speed_raw=pkt->data.navvelned.gSpeed; //cm/s
    	currentGPSData.vspeed_raw=pkt->data.navvelned.velD; //cm/s
    	currentGPSData.heading_raw=pkt->data.navvelned.heading; // /1e5 deg
    	return;
    }
    if (pkt->header.messageClass == 0x05 && pkt->header.messageId == 0x01){//ACK
      ack_received = 1;
      debug("\rACK\n");
      return;
    }
    if (pkt->header.messageClass == 0x05 && pkt->header.messageId == 0x00){//NACK
      nack_received = 1;
      debug("\rNACK\n");
      return;
    }


}
uint8_t ublox_wait_for_ack() {
  ack_received = 0;
  nack_received = 0;
  uint8_t timeout = 200;
  while(!ack_received && !nack_received){
    _delay_ms(1);
    if (!timeout--){
    	return 0;
      break;
    }
  }

  return ack_received;
}


