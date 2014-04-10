#include "game.h"
#include "wrapper.h"
#include "sky.h"

#define GROUND
#define RUMBLES
#define CURVED
#define SEGMENTS
//#define ROADSIDE
//#define TRAFFIC

int zlookup[YTABS],xlookup[YTABS];
int curvature[YTABS+5];
int xspots[NUMSPOTS];
int zspots[NUMSPOTS];
int xonscreen[NUMSPOTS];
int yonscreen[NUMSPOTS];
int xcars[NUMCARS];
int zcars[NUMCARS];
int drawn=0;

unsigned long z_world=0,nextz,nexto;
int z_car=0,zspeed = 0, wheeltick=0, carx=TVXCENTER-16,cary=CARY,
    acceltick=ACCELTIME, deceltick=1,fumeframe=0,roadx=0,skytick=0,
    skyx=0,segment=0,osegment=0,curvtime=0,curvcount=0,xaccel=0,dynamichz=HORIZON;

int nexthill,nexthz,hilloffset=256;
byte lanes=3,car_dir=UP,lastRoad,segvisible=false,curveover=true,hillstate;
signed char wheeloffset=0;


//=========================================================================
// Draw sky
//=========================================================================

void drawSky(int x,int y){
     TV.bitmap(x, y, sky);
}

//=========================================================================
// BUILD ROAD GEOMETRY
//=========================================================================

void gameSetup() {
    int k=0;
    lastRoad=track1[0];
    // table of spots
    for (int i=0; i < NUMSPOTS; i+=2) {
        xspots[i] = 22;   // distributed PALMS !!
        xspots[i+1] = -22;//*-8;   // distributed PALMS !!
        zspots[i] = (z_car)+i*(1000/NUMSPOTS);
        zspots[i+1] = (z_car)+i*(1000/NUMSPOTS);
    }
    k=0;

    // table for cars
    for (int i=0; i < NUMCARS; i++) {
        xcars[i] = ROADW;   // distributed cars
        zcars[i] = (i+1)*(1000);
    }
    zcars[0] = 400;
    zcars[1] = 800;
    zcars[2] = 1000;
    zcars[3] = 1300;
    xcars[0] = 25;
    xcars[1] = -35;
    xcars[2] = -5;
    xcars[3] = -35;

    // lookup table for z and road sides
    float ztemp, yxtra=1;
    for (int y=TVY-1; y>=dynamichz-XTRA ;y--) {
        int zy=dynamichz;
        if (y - dynamichz > 0) {zy = Y_CAMERA / (y-dynamichz);
        } else { yxtra *= 1.5; zy=Y_CAMERA*yxtra; }
        zlookup[k] = zy;
        ztemp = (((float)ROADW/(float)zy)*(float)Z_MULT);
        xlookup[k] = 2*ztemp;
        k++;
    }
    // road curvature table

    for (k=0; k<YTABS+5;k++) {
        curvature[k] = k*k/500; // was 300
    }

}

//=========================================================================
// checkSegment - check for changes in road structure
//=========================================================================

void checkSegment() {

}

//=========================================================================
// DRAW ROAD
//=========================================================================

void drawRoad() {
    unsigned int ztemp, easeout = true;
    int yTransition = dynamichz,curvacceltop=0,curvaccelbot=0,curvoffset=0;
    int q_step,q_pointer;


    // determine segment transition points
    ztemp = (z_world >> SEGSLOWDOWN) & (SEGLENGTH-1);
    ztemp = SEGLENGTH - ztemp; // segment edges otherwise come too late

    //check if transition points are within visible range
    if (!segvisible && ztemp > 900) { //was 600 and 800
        segvisible = true;
        yTransition = dynamichz;
        nextz = z_world+ZTICKER*3;
        nexto = z_world+ZTICKER;
    }

    // determine hill state
    // seg  seg+1   seg+2   equals
    // flat flat    flat    FLATLAND
    // flat flat    up      HILLCOMING
    // flat up      any     UPSLOPE
    // up   up      any     UPSLOPE
    // up   flat/down any   CREST
    // flat flat    down    DROPCOMING
    // flat down    any     DROPCOMING2
    // down flat    any     DOWNSLOPE
    // down up      any     VALLEY

    hillstate=FLATLAND;
    if (!(track1[segment]&UPHILL) && !(track1[segment]&DOWNHILL)) {
        // current segment FLAT
        if (!(track1[segment+1]&UPHILL) && !(track1[segment+1]&DOWNHILL)) {
            // next segment also FLAT
            hillstate = FLATLAND; // unless changed, is FLAT
            if (track1[segment+2]&UPHILL) hillstate = HILLCOMING;
            if (track1[segment+2]&DOWNHILL) hillstate = DROPCOMING;
        }

        if (track1[segment+1]&UPHILL) hillstate = UPSLOPE;
        if (track1[segment+1]&DOWNHILL) hillstate = DROPCOMING2;
    } else {
        // current segment is NOT FLAT
        if (track1[segment]&DOWNHILL) {
                hillstate = DOWNSLOPE; // unless it goes straight to upslope
                if (track1[segment+1]&UPHILL) hillstate = VALLEY;
            } else {
                hillstate = CREST; // crest, unless
                if (track1[segment+1]&UPHILL) hillstate = UPSLOPE;
            }
    }


    switch (hillstate) {
    case HILLCOMING:
        nexthz = HORIZON - 3;
        nexthill =250;
        break;
    case FLATLAND:
        nexthz = HORIZON;
        nexthill=256;
        break;
    case UPSLOPE:
        nexthz = HORIZON - 11;
        nexthill=210;
        break;
    case CREST:
        nexthz = HORIZON + 6;
        nexthill=512;
        break;
    case DROPCOMING:
        nexthz = HORIZON + 3;
        nexthill=400;
        break;
    case DROPCOMING2:
        nexthz = HORIZON + 2;
        nexthill=512;
        break;
    case DOWNSLOPE:
        nexthz = HORIZON ;
        nexthill=300;
        break;
    case VALLEY:
        nexthz = HORIZON -4 ;
        nexthill=300;
        break;
    default:
        nexthz = HORIZON;
        nexthill=256;
    }

    if (nexthz != dynamichz && z_world > nextz) {
        if (dynamichz < nexthz) dynamichz++;
        if (dynamichz > nexthz) dynamichz--;
        nextz = z_world+ZTICKER;
    }

    if (nexthill != hilloffset && z_world > nexto) {
        if (hilloffset < nexthill) {
            hilloffset+=5;
            if (hilloffset > nexthill) hilloffset = nexthill;
            } else if (hilloffset > nexthill) {
            hilloffset-=5;
            if (hilloffset < nexthill) hilloffset = nexthill;
            }

        nexto = z_world+ZTICKER;
    }


    if (segvisible) { // was 512
        yTransition = dynamichz+1+(SEGLENGTH-ztemp)/8;
        q_step = YTABS;
        q_step *= 256; // some weird compiler bug
        q_step = q_step/(yTransition-dynamichz);
        // reset transition
        if (yTransition >= TVY) {
            segment++;
            segvisible = false;
            yTransition = dynamichz;
        }
    }


    // set up curve counter
    if (track1[segment]&CURVERIGHT) {curvaccelbot=1;}
    if (track1[segment]&CURVELEFT) {curvaccelbot=-1;}
    if (track1[segment+1]&CURVERIGHT) {curvacceltop=1;}
    if (track1[segment+1]&CURVELEFT) {curvacceltop=-1;}

    if (curvacceltop == curvaccelbot) easeout = false; // disable easeout

    // effect of road on car
    if (yTransition < TVY-25) {
        roadx += curvaccelbot*(zspeed/100);
        } else if (yTransition < TVY-15) roadx += curvaccelbot*(zspeed/200);



    // draw ground
    #ifdef GROUND
    for (int screeny=TVY-1; screeny > dynamichz; screeny--) {
        int a,a2,b,b2,c,d,dx,zy,rumblew,poffset;
        int q = TVY-screeny; // define index for lookup

        poffset = roadx*xlookup[q]/DXDIV;
        if (screeny < yTransition && easeout) {
            // road ABOVE transition
            q_pointer = ((yTransition-screeny+1)*q_step)/256;
            if(q_pointer>YTABS) q_pointer=YTABS;
            curvoffset += curvacceltop*curvature[q_pointer];
            q_pointer -= 6;
            if (q_pointer<0) q_pointer =0;
        } else {
            // road BELOW transition
            curvcount = yTransition - dynamichz;
            if (easeout) q_pointer = q-curvcount;
            else q_pointer=q;
            if (q_pointer<0) q_pointer = 0;
            curvoffset += curvaccelbot*curvature[q_pointer];
        }

        q=q*hilloffset/256;

        a = TVXCENTER - xlookup[q] + poffset + curvoffset;
        a2 = TVXCENTER - (xlookup[q]>>1) + poffset + curvoffset;
        b = TVXCENTER + xlookup[q] + poffset + curvoffset;
        b2 = TVXCENTER + (xlookup[q]>>1) + poffset + curvoffset;

        // Clean the road
        if (a>=0 && b <=TVX) {
                if (screeny > cary + 14 && screeny < cary + 19 ) {
                    TV.draw_line (a,screeny,carx-14,screeny,0);
                    TV.draw_line (carx+60,screeny,b,screeny,0);
                } else {
                TV.draw_line (a,screeny,b,screeny,0);
                }
        } else {
                TV.draw_line (0,screeny,TVX,screeny,0);
        }

         // check if a or b are out of bounds and draw road sides
        if (a>=0) TV.draw_line (0,screeny,a,screeny,1);
        if (a>TVX) TV.draw_line (0,screeny,TVX,screeny,1);
        if (b<=TVX) TV.draw_line (b,screeny,TVX,screeny,1);
        if (b<0) TV.draw_line (0,screeny,TVX,screeny,1);

        // draw rumbles
        zy = zlookup[q];
        //if (zy<1) break; // invalid line
        zy += z_car;
        byte col = colorlookup[zy&63]; // z_car modulus 64 x%64 == x&63 to have only a 64 byte rumble buffer
        dx = xlookup[q]; // find dx width for this Z depth
        rumblew = dx/RUMBLEW;
        a = a + rumblew; // rumble left edge
        c = b - rumblew; // right side rumble right edge
        d = c - rumblew; // right side rumble left edge
        b = a + rumblew; // rumble right edge
        if (a<0) a=0; // rumble left edge out of screen check
        if (b>0) TV.draw_line (a,screeny,b,screeny,col); // if rumble is visible, draw
        if (c>TVX) c=TVX; // check if roadside is out of screen
        if (d<TVX) TV.draw_line (c,screeny,d,screeny,col); // if visible, draw
        a = a2 + 2.5 *rumblew; // rumble left edge
        c = b2 - 2.5 *rumblew; // right side rumble right edge
        d = c - rumblew; // right side rumble left edge
        b = a + rumblew; // rumble right edge
        if (a<0) a=0; // rumble left edge out of screen check
        if (b>0) TV.draw_line (a,screeny,b,screeny,col); // if rumble is visible, draw
        if (c>TVX) c=TVX; // check if roadside is out of screen
        if (d<TVX) TV.draw_line (c,screeny,d,screeny,col); // if visible, draw

    }
    #endif

    #ifdef SEGMENTS
        if (yTransition > dynamichz && yTransition <TVY ) {
        TV.draw_line(0,yTransition,TVX,yTransition,1);
        }
    #endif // SEGMENTS

    #ifdef ROADSIDE
    //draw spots
    for (int i=0; i < NUMSPOTS; i++) {
        int y,x,z,coffset;
        x = xspots[i];
        z = zspots[i];

        if (z < z_world+20) {
            zspots[i] = z_world+ 1000; // push spot to dynamichz
            x = xspots[i];
            z = zspots[i];
        }

        y = (Y_CAMERA / (z-z_world)) + dynamichz;
        #ifdef CURVED
            coffset = curvature[TVY-y];
        #else
            coffset=0;
        #endif
            if (z-z_world > 650) {
                    //alphabitmap(x-3,y-7,palm_tiny,palm_tiny_alpha);
                } else {
                if (z-z_world > 500) {
                    //x = TVXCENTER+(roadx*dxlookup[TVY-y-2]/DXDIV)+3*x*dxlookup[TVY-y-2]/DXDIV+coffset;
                    //if(x>3 && x < TVX-10) alphabitmap(x-3,y-7,palm_tiny,palm_tiny_alpha);
                } else {
                    if (z-z_world > 250) {
                         x = TVXCENTER+(roadx*dxlookup[TVY-y-1]/DXDIV)+3*x*dxlookup[TVY-y-1]/DXDIV+coffset;
                        if(x>3 && x < TVX-10) alphabitmap(x-3,y-8,palm_tiny,palm_tiny_alpha);
                        } else {
                            if (z-z_world > 205) {
                                x = TVXCENTER+(roadx*dxlookup[TVY-y-3]/DXDIV)+3*x*dxlookup[TVY-y-3]/DXDIV+coffset;
                                if(x>3 && x < TVX-10) alphabitmap(x-3,y-10,palm_small,palm_small_alpha);
                            } else {
                                if (z-z_world > 150) {
                                        x = TVXCENTER+(roadx*dxlookup[TVY-y-2]/DXDIV)+3*x*dxlookup[TVY-y-2]/DXDIV+coffset;
                                        if(x>4 && x < TVX-10) alphabitmap(x-4,y-15,palm,palm_alpha);
                                        } else {
                                        if (z-z_world > 30) {
                                                x = TVXCENTER+(roadx*dxlookup[TVY-y-6]/DXDIV)+3*x*dxlookup[TVY-y-6]/DXDIV+coffset;
                                                if(x>10 && x < TVX-10) alphabitmap(x-6,y-22,palm_big,palm_big_alpha);
                                                }
                                        }}}}}
}
    #endif // ROADSIDE


    #ifdef TRAFFIC
            // draw cars

   for (int i=NUMCARS; i >-1; i--) {
        int y,x,z,coffset;
        x = xcars[i];
        zcars[i] += OTHERCARSSPEED;
        z = zcars[i];

        if (z < z_world-30) {
            zcars[i] = z_world+ 1000*NUMCARS; // push spot to dynamichz
            x = xcars[i];
            z = zcars[i];
        }
        if(z-z_world==0) z++;
        y = (Y_CAMERA / (z-z_world)) + dynamichz;
        #ifdef CURVED
            coffset = curvature[TVY-y-5];
        #else
            coffset=0;
        #endif
            if (z-z_world > 2000) {
            } else {
            if (z-z_world > 1500) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-3];
                    #else
                    coffset=0;
                    #endif
                    x = TVXCENTER+(roadx*dxlookup[TVY-y-3]/DXDIV)+x*dxlookup[TVY-y-3]/DXDIV+coffset;
                    if(x>0 && x < TVX-10)TV.draw_rect(x,y+2,2,1,0,0);
            } else {
            if (z-z_world > 1100) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-3];
                    #else
                    coffset=0;
                    #endif
                    x = TVXCENTER+(roadx*dxlookup[TVY-y-3]/DXDIV)+x*dxlookup[TVY-y-3]/DXDIV+coffset;
                    if(x>0 && x < TVX-10)TV.draw_rect(x,y+1,3,2,0,1);
            } else {
            if (z-z_world > 500) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-5];
                    #else
                    coffset=0;
                    #endif
                     x = TVXCENTER+(roadx*dxlookup[TVY-y-5]/DXDIV)+x*dxlookup[TVY-y-5]/DXDIV+coffset;
                    if(x>0 && x < TVX-10)alphabitmap(x-1,y,porsche_tiny,porsche_tiny_alpha);
            } else {
            if (z-z_world > 200) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-8];
                    #else
                    coffset=0;
                    #endif
                    x = TVXCENTER+(roadx*dxlookup[TVY-y-8]/DXDIV)+x*dxlookup[TVY-y-8]/DXDIV+coffset;
                    if(x>0 && x < TVX-10)alphabitmap(x,y,porsche_small,porsche_small_alpha);
            } else {
            if (z-z_world > 30) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-9];
                    #else
                    coffset=0;
                    #endif
                    x = TVXCENTER+(roadx*dxlookup[TVY-y-9]/DXDIV)+x*dxlookup[TVY-y-9]/DXDIV+coffset;
                    if(x>0 && y < TVY-21 && x< TVX-10) alphabitmap(x,y,porsche_med,porsche_med_alpha);
            }}}}}}}
    #endif // TRAFFIC
}
