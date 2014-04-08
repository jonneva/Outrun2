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

unsigned long z_world=0;
int z_car=0,zspeed = 0, wheeltick=0, carx=TVXCENTER-16,cary=CARY,
    acceltick=ACCELTIME, deceltick=1,fumeframe=0,roadx=0,skytick=0,
    skyx=0,segment=0,osegment=0,curvtime=0,curvcount=0;


byte lanes=3,car_dir=UP,lastRoad,segvisible;
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
    float ztemp;
    for (int y=TVY-1; y>=HORIZON ;y--) {
        int zy=HORIZON;
        if (y - HORIZON > 0) {zy = Y_CAMERA / (y-HORIZON);
        } else { break;}
        zlookup[k] = zy;
        ztemp = (((float)ROADW/(float)zy)*(float)Z_MULT);
        xlookup[k] = 2*ztemp;
        k++;
    }
    // road curvature table

    for (k=0; k<YTABS+5;k++) {
        curvature[k] = k*k/300;
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
    unsigned int ztemp,zapex;
    int yTransition = HORIZON,curvacceltop=0,curvaccelbot=0,curvoffset=0;
    int q_step,q_pointer,yApex;

    // determine segment transition points
    ztemp = (z_world >> SEGSLOWDOWN) & (SEGLENGTH-1);
    ztemp = SEGLENGTH - ztemp;

    //check if transition points are within visible range
    if (ztemp > 600 && ztemp < 800) segvisible = true;
    if (ztemp < 512 && segvisible) { // was 512
        yTransition = HORIZON + (Y_CAMERA / (ztemp+1));
        q_step = YTABS;
        q_step *= 256; // some weird compiler bug
        q_step = q_step/(yTransition-HORIZON-1);
        if (yTransition >= TVY) {
            segment++;
            segvisible = false;
            yTransition = HORIZON;
        }
    }

    // set up curve counter
    if (track1[segment]&CURVERIGHT) {curvaccelbot=1;}
    if (track1[segment]&CURVELEFT) {curvaccelbot=-1;}
    if (track1[segment+1]&CURVERIGHT) {curvacceltop=1;}
    if (track1[segment+1]&CURVELEFT) {curvacceltop=-1;}

    // draw ground
    #ifdef GROUND
    for (int screeny=TVY-1; screeny > HORIZON; screeny--) {
        int a,a2,b,b2,c,d,dx,zy,rumblew,poffset;
        int q = TVY-screeny; // define index for lookup

        poffset = roadx*xlookup[q]/DXDIV;
        if (screeny < yTransition) {
            q_pointer = ((yTransition-screeny+1)*q_step)/256;
            if(q_pointer>YTABS) q_pointer=YTABS;
            curvoffset += curvacceltop*curvature[q_pointer];
        } else {
            q_pointer=q;
            curvoffset += curvaccelbot*curvature[q_pointer];
            curvcount = ztemp;
        }


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
        if (b<=TVX) TV.draw_line (b,screeny,TVX,screeny,1);

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
        if (yTransition > HORIZON && yTransition <TVY ) {
        TV.draw_line(0,yTransition,TVX,yTransition,1);
        }
        if (yApex > HORIZON && yApex <TVY ) {
        TV.draw_line(0,yApex,TVX,yApex,1);
        }
    #endif // SEGMENTS

    #ifdef ROADSIDE
    //draw spots
    for (int i=0; i < NUMSPOTS; i++) {
        int y,x,z,coffset;
        x = xspots[i];
        z = zspots[i];

        if (z < z_world+20) {
            zspots[i] = z_world+ 1000; // push spot to horizon
            x = xspots[i];
            z = zspots[i];
        }

        y = (Y_CAMERA / (z-z_world)) + HORIZON;
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
            zcars[i] = z_world+ 1000*NUMCARS; // push spot to horizon
            x = xcars[i];
            z = zcars[i];
        }
        if(z-z_world==0) z++;
        y = (Y_CAMERA / (z-z_world)) + HORIZON;
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
