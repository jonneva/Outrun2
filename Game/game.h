#ifndef GAME_H
#define GAME_H

#include "wrapper.h"

// GRAPHICS SETUP CONSTANTS

#define TVX         104     // 112x56 = 6272 bytes 104x56 = 5824
#define TVY         56      // 136x96 = 13056 bytes
#define TVXCENTER   TVX/2   // 120x64 = 7680 bytes
#define TVYCENTER   TVY/2
#define HORIZON     12  //TVYCENTER+5
#define PARALLAXTOP 6   //HORIZON-PARALLAXTOP is shifted area

// GAME SETUP CONSTANTS

#define SEGLENGTH   1024   // segment length = 2048
#define SEGSLOWDOWN 0       // divide world z progress by 2^SEGSLOWDOWN to slow segment change
#define ZLINES      2  // number of rumble segments
#define ZSTEP       32  // length of rumble segments
#define DXDIV       32  // Number of steps from centerline to roadside
#define NUMSPOTS    16  // Number of roadside spots flying by
#define ACCELTIME   60  // Time that wheels are "smoking"
#define CARY        TVY-25
#define NUMCARS     4
#define OTHERCARSSPEED 15

#define XTRA        5       // Extra length for tables
#define ZTICKER     20     // counter for hill transition speed
#define ROADW       1600    // Road width in worldspace
#define Y_CAMERA    1140    // Camera height for lookup table calculation, was 1140
#define Z_MULT      2       // Z multiplier, was 2
#define YTABS       TVY-HORIZON+XTRA   // lookup table sizes = how many y lines are checked
#define RUMBLEW     12      // Rumble width, higher number = thinner
//#define RUMBLEOFFS  12      // define rumble offset from road side, higher = less


// GAME ROAD DEFINITIONS
#define STRAIGHT        0x00    // bit 1 NOT = straight
#define CURVERIGHT      0x01    // bit 1 set = right curve
#define CURVELEFT       0x02    // bit 2 set = left curve
#define STEEPCURVE      0x04    // bit 3 set = steep curve
#define SHALLOWCURVE    0x08    // bit 4 set = shallow curve
#define NOHILL          0x00    // bit 5 NOT = level road
#define UPHILL          0x10    // bit 5 set = uphill
#define DOWNHILL        0x20    // bit 6 set = downhill
#define NORMALROAD      0x00    // bit 7 NOT = normal road
#define WIDEROAD        0x40    // bit 7 set = wide road
#define SPLITROAD       0x80    // bit 8 set = split road

// HILL STATE DEFINITIONS
#define FLATLAND        0       // no hills visible
#define HILLCOMING      1       // hill visible at segment + 2
#define UPSLOPE         2       // going uphill
#define CREST           3       // on hill crest
#define DROPCOMING      4       // on hill crest
#define DROPCOMING2     5       // on hill crest
#define DOWNSLOPE       6       // going downhill towards flat
#define VALLEY          7       // transition immediately to upslope


// GAME LOOKUP TABLES

#ifdef ARDUINO
extern prog_uchar colorlookup[];
extern prog_uchar track1[];
#else
extern byte colorlookup[];
extern byte track1[];
#endif // ARDUINO

extern int zlookup[],xlookup[];
extern int xspots[NUMSPOTS];
extern int zspots[NUMSPOTS];
extern int xcars[NUMCARS];
extern int zcars[NUMCARS];
extern int xonscreen[NUMSPOTS];
extern int yonscreen[NUMSPOTS];

// GAME VARIABLES

extern unsigned long z_world;
extern int  z_car, zspeed, wheeltick, carx,cary,acceltick,
            deceltick, fumeframe, roadx,skytick,skyx,skyy,segment, osegment,
            curvtime,curvcount,xaccel,dynamichz;
extern byte lanes,car_dir,hillstate;
extern signed char wheeloffset;

// GAME FUNCTIONS

extern void gameSetup();
extern void drawSky(int,int);
extern void drawRoad();
#endif
