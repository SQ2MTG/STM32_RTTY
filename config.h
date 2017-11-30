#ifndef RS41HUP_CONFIG_H
#define RS41HUP_CONFIG_H

// Created by SQ5RWU on 2016-12-27.
// modified by SQ6QV in November 2017


#define CALLSIGN "NOCALL"

#define SEND_INTERVAL 60000


//RTTY custom shift 544 Hz, baudrate 50, 7bit  parity none, one stop bit

//uncomment to enable RTTY beacon
//#define RTTY_BEACON 432.500
#define RTTY_BAUDRATE 50



//uncomment to enable audio output on STM32 PA4
#define AUDIO_APRS_BEACON

//uncomment to enable APRS on internal radio
//#define APRS_BEACON 432.500

//preable time in ms (800ms Baofeng)
#define APRS_PREAMBLE 800


#define APRS_CALLSIGN CALLSIGN
#define APRS_SSID 14
//#define APRS_SYMBOL APRS_SYM_CAR
#define APRS_SYMBOL APRS_SYM_BALLOON
#define APRS_PATH "WIDE1-1,WIDE2-2"



#define APRS_DEST_CALLSIGN				"APZQVA"
#define APRS_DEST_SSID					0

//uncomment to enable vertical velocity/altitude data
#define HAB_USE


//uncomment to enable supply voltage readout in APRS frames
#define APRS_VOLTAGE


#define APRS_SYM_BALLOON					0x2F4F
#define APRS_SYM_SMALLAIRCRAFT				0x2F27
#define APRS_SYM_SATELLITE					0x5C53
#define APRS_SYM_CAR                        0x2F3E
#define APRS_SYM_SHIP                       0x2F73


#define DEFAULT_TX_POWER 7





//**************RTTY Data Format**********************
// $$<callsign>,<frame>,<hhmmss>,<latitude>,<longitude>,<height>,<radio chip temperature (°C)>,<battery voltage>,<used gps satellites>,<good gps datasets>,<bad gps datasets>,<gps fix>,<CRC>


//**************config**********************
// 0 --> Your primary station usually fixed and message capable
// 1 --> generic additional station, digi, mobile, wx, etc.
// 2 --> generic additional station, digi, mobile, wx, etc.
// 3 --> generic additional station, digi, mobile, wx, etc.
// 4 --> generic additional station, digi, mobile, wx, etc.
// 5 --> Other network sources (Dstar, Iphones, Blackberry's etc)
// 6 --> Special activity, Satellite ops, camping or 6 meters, etc.
// 7 --> walkie talkies, HT's or other human portable
// 8 --> boats, sailboats, RV's or second main mobile
// 9 --> Primary Mobile (usually message capable)
// A --> internet, Igates, echolink, winlink, AVRS, APRN, etc.
// B --> balloons, aircraft, spacecraft, etc.
// C --> APRStt, DTMF, RFID, devices, one-way trackers*, etc.
// D --> Weather stations
// E --> Truckers or generally full time drivers
// F --> generic additional station, digi, mobile, wx, etc.

#define APRS_COMMENT " RS41 tracker"
#define RTTY_TO_APRS_RATIO 5 //transmit APRS packet with each x RTTY packet

//*************TX Frequencies********************
#define RTTY_FREQUENCY  434.500f //Mhz middle frequency
#define APRS_FREQUENCY  432.500f //Mhz middle frequency

//************RTTY Shift*********************** si4032
#define RTTY_DEVIATION 0x2	// RTTY shift = RTTY_DEVIATION x 270Hz

//************rtty bits************************ si4032
#define RTTY_7BIT   1 // if 0 --> 5 bits

//************rtty stop bits******************* si4032
//#define RTTY_USE_2_STOP_BITS

//********* power definition**************************
//#define TX_POWER  0 // PWR 0...7 0- MIN ... 7 - MAX
// 0 --> -1dBm
// 1 --> 2dBm
// 2 --> 5dBm
// 3 --> 8dBm
// 4 --> 11dBm
// 5 --> 14dBm
// 6 --> 17dBm
// 7 --> 20dBm
//****************************************************


#endif //RS41HUP_CONFIG_H
