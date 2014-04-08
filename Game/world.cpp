#ifdef ARDUINO
#include <Arduino.h>
#endif
#include "game.h"
// switch PROGMEM prog_uchar with unsigned char
#ifdef ARDUINO
PROGMEM prog_uchar colorlookup[] = {
#else
unsigned char colorlookup[] = {
#endif
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

#ifdef ARDUINO
PROGMEM prog_uchar track1[] = {
#else
unsigned char track1[] = {
#endif
0x00,0x02,0x00,0x01,0x00,0x01,0x02,0x00,
0x00,0x01,0x02,0x00,0x01,0x00,0x00,0xFF
};
