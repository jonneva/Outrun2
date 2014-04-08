// SFML_TVout ... port of TVout library to SFML / Jonne Valola

#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include "wrapper.h"
#include "TVout.h"
#include "fontALL.h"
#include "game.h"
#include "ferrari.h"
#include "sky.h"

using namespace std;
using namespace sf;

// START OF ARDUINO CODE

#define GROUND
#define SKY
#define RUMBLES

#include <Controllers.h>

void setup() {
    TV.begin(_PAL, viewWidth, viewHeight);
    TV.select_font(font8x8);
    gameSetup();

    #ifdef SKY
        TV.bitmap(0, 14, sky);
    #endif // SKY
}

void loop() {
    int xtemp,zlook;
    TV.delay_frame(10);
    TV.draw_line(0,49,viewWidth,49,1);
    TV.print(viewWidth/2+25+1*8,6,"   ");
    if (zspeed < 10) TV.print(viewWidth/2+25+3*8,6,zspeed);
    else if (zspeed < 100) TV.print(viewWidth/2+25+2*8,6,zspeed);
    if (zspeed > 100) TV.print(viewWidth/2+25+1*8,6,zspeed);

    // draw ground
    #ifdef GROUND
    for (int screeny=95; screeny > 49; screeny--) {
        int a,b;
        int q = viewHeight-screeny; // define index for lookup

        // find road sides
        a = x1lookup[q] + roadx*dxlookup[q]/DXDIV;
        b = x2lookup[q] + roadx*dxlookup[q]/DXDIV;

        // Clean the road
        if (a>=0 && b <=viewWidth) {
                if (screeny > cary + 14 && screeny < cary + 19 ) {
                    TV.draw_line (a,screeny,carx-14,screeny,0);
                    TV.draw_line (carx+60,screeny,b,screeny,0);
                } else {
                TV.draw_line (a,screeny,b,screeny,0);
                }
        } else {
                TV.draw_line (0,screeny,viewWidth,screeny,0);
        }

        // check if a or b are out of bounds and draw road sides

        if (a>=0) TV.draw_line (0,screeny,a,screeny,1);
        if (b<=viewWidth) TV.draw_line (b,screeny,viewWidth,screeny,1);
    }
    #endif

    //draw spots
    for (int i=0; i < NUMSPOTS; i++) {
        int y,x,z;
        x = xspots[i];
        z = zspots[i];
        // delete the spot
        if (xonscreen[i]>-1) TV.set_pixel (xonscreen[i],yonscreen[i],1);
        // see if spot is out of range
        if (z < z_spots+20) {
            // spot has moved out of sight, push back to horizon
            xspots[i] = random(ROADW+50,4000)*Z_MULT;             // distributed round road
            if (random(0,4)<2) xspots[i] *= -1; // distribute on both sides
            zspots[i] = z_spots+ 600; // push spot to horizon
            x = xspots[i];
            z = zspots[i];
        }
        y = (Y_CAMERA / (z-z_spots)) + (viewHeight/2);
        x = x / (z-z_spots) + (viewWidth/2)+(roadx*dxlookup[viewHeight-y]/DXDIV);
        if (x>0 && x < viewWidth) {
                TV.set_pixel (x,y,0);
                xonscreen[i]=x;
                yonscreen[i]=y;
            } else {
                xonscreen[i]=-1; // no need to wipe
            }
        }

    // draw rumbles
    #ifdef RUMBLES
    for (int screeny=94; screeny > viewHeight/2; screeny--) {
        int a,b,c,d,dx,zy,rumblew,roffset;
        int q = viewHeight-screeny;
        zy = zlookup[q];
        if (zy<1) break; // invalid line
        byte col = colorlookup[zy+z_car];
        dx = dxlookup[q]; // find dx width for this Z depth
        rumblew = dx/RUMBLEW;
        roffset = dx/RUMBLEOFFS;
        a = x1lookup[q]+(roadx*dx/DXDIV)+ roffset; // rumble left edge
        b = a+rumblew; // rumble right edge
        if (a<0) a=0; // rumble left edge out of screen check
        if (b>0) TV.draw_line (a,screeny,b,screeny,col); // if rumble is visible, draw
        c = x2lookup[q]+(roadx*dx/DXDIV) - roffset; // right ride of road
        d = c-rumblew; // rumble left edge
        if (c>viewWidth) c=viewWidth; // check if roadside is out of screen
        if (d<viewWidth) TV.draw_line (c,screeny,d,screeny,col); // if visible, draw
    }
    #endif
    // draw car
    overlaybitmap(carx,cary, ferrari_bitmap, 0, 0, 0);
    if (Controller.upPressed()) {
            if (acceltick>0) {
                if (fumeframe == 0) TV.bitmap(carx-14,cary+14,fumes1);
                if (fumeframe == 3) TV.bitmap(carx-14,cary+14,fumes2);
                if (fumeframe == 9) TV.bitmap(carx-14,cary+14,fumes3);
                fumeframe++;
                if (fumeframe == 12) fumeframe=0;
                acceltick--;
                if (acceltick == 0) {
                    //TV.draw_rect(carx-14,cary+14,carx+46,cary+19,0,0);
                    //overlaybitmap(carx,cary, ferrari_bitmap, 0, 0, 0);
                }
            }
            zspeed+=10;
    } else {
        if (deceltick) zspeed-=4;
        deceltick = !deceltick;
        if (acceltick != 30 ) {
        acceltick = ACCELTIME;
        fumeframe=0;
        //TV.draw_rect(carx-14,cary+14,carx+46,cary+19,0,0);
        //overlaybitmap(carx,cary, ferrari_bitmap, 0, 0, 0);
        }
    }
    if (Controller.downPressed()) zspeed-=15;
    if (zspeed<0) zspeed = 0;
    if (zspeed>293) zspeed = 293;
    if (Controller.leftPressed()) {
            roadx+=(1+zspeed/100);
            }
    if (Controller.rightPressed()) {
            roadx-=(1+zspeed/100);
    }
    z_car += zspeed/10 ;
    z_spots += zspeed/5 ;
    if (z_car>800) z_car=200;
    if (zspeed>10) {
            if (wheeltick == 0) TV.bitmap(carx, cary+17, wheels1);
            if (wheeltick == 1) TV.bitmap(carx, cary+17, wheels2);
            if (wheeltick == 2) {
                    TV.bitmap(carx, cary+17, wheels3);
                    wheeltick = 0;
            } else { wheeltick++; }
    }
}


// END OF ARDUINO CODE

int main()
{
    std::cout << "SFML TVout emulator started" << std::endl;
	TVsetup();
    setup();

while (window.isOpen())
    {
	loop();
    }
	return 0;
}

