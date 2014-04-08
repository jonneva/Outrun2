// SFML_TVout ... port of TVout library to SFML / Jonne Valola

#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include "wrapper.h"
#include "TVout.h"
#include "fontALL.h"
#include "game.h"

using namespace std;
using namespace sf;

// START OF ARDUINO CODE

#define ZLINES      20
#define ZSTEP       50
#define XBARDIV     8
#define NUMSPOTS    5

int Y_screen, roadw=1000,Y_ground=0, Y_camera=1140, Z[ZLINES],
    z_mult=4,z_cam=0,z=0,zy,z_car=200,z_spots=200,z_min=30, z_max=200,
    zspeed = 0;

int zlookup[59],x1lookup[59],x2lookup[59],xbarlookup[59],looky=95;
byte colorlookup[ZLINES*ZSTEP];
int xspots[NUMSPOTS];
int zspots[NUMSPOTS];
int xonscreen[NUMSPOTS];
int yonscreen[NUMSPOTS];

#include <Controllers.h>

void setup() {
    int k=0;
    // color lookup table & random crud on roadside
    for (int i=0; i< ZLINES; i++) {
            for (int j=0; j< ZSTEP;j++)
            {
                 colorlookup[i*ZSTEP+j]=(byte)k;
            }
            if (k==1) {
                    k=0;
            }else {
                    k=1;
            }
    }
    // table of spots
    for (int i=0; i < NUMSPOTS; i++) {
        xspots[i] = random(roadw,4000)*z_mult;             // distributed round road
        if (random(0,4)<2) xspots[i] *= -1; // distribute on both sides
        zspots[i] = (z_car)+i*(600/NUMSPOTS);
    }
    k=0;

    // lookup table for z and road sides
    for (int y=95; y>viewHeight/2;y--) {
        zy = Y_camera / (y-(viewHeight/2));
        zlookup[k] = zy;
        if (zy < 0) break;
        x1lookup[k] = viewWidth/2 - ((roadw/zy)*z_mult);
        x2lookup[k] = viewWidth/2 + ((roadw/zy)*z_mult);
        xbarlookup[k] = (((roadw/zy)*z_mult)/XBARDIV)+1;
        k++;
    }
    TV.begin(_PAL, viewWidth, viewHeight);
    TV.select_font(font4x6);
        // draw ground
    for (int screeny=95; screeny > 49; screeny--) {
        int a,b;
        int q = viewHeight-screeny;
        a = x1lookup[q];
        b = x2lookup[q];
        if (a>=0) TV.draw_line (0,screeny,a-xbarlookup[q],screeny,1);
        if (b<=viewWidth) TV.draw_line (b+xbarlookup[q],screeny,viewWidth,screeny,1);
    }
}

void loop() {
    int xtemp,zlook;
    TV.delay_frame(1);
    //TV.clear_screen();
    TV.draw_line(0,49,viewWidth,49,1);
    TV.print(0,0,"Y_cam: ");
    TV.print(Y_camera);
    TV.print(0,6,"Z_cam: ");
    TV.print(z_cam);
    TV.print(viewWidth/2,6,"Z_car: ");
    TV.print(z_car);

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
            xspots[i] = random(roadw,4000)*z_mult;             // distributed round road
            if (random(0,4)<2) xspots[i] *= -1; // distribute on both sides
            zspots[i] = z_spots+ 600; // push spot to horizon
            x = xspots[i];
            z = zspots[i];
        }
        y = (Y_camera / (z-z_spots)) + (viewHeight/2);
        x = x / (z-z_spots) + (viewWidth/2);
        if (x>0 && x < viewWidth) {
                TV.set_pixel (x,y,0);
                xonscreen[i]=x;
                yonscreen[i]=y;
            } else {
                xonscreen[i]=-1; // no need to wipe
            }
        }

    // draw rumbles
    for (int screeny=95; screeny > viewHeight/2; screeny--) {
        int a,b,c,d;
        int q = viewHeight-screeny;
        zlook = zlookup[q];
        if (zlook<1) break;
        zy = zlook;
        byte col = colorlookup[zy+z_car];
        a = x1lookup[q];
        b = x1lookup[q]+xbarlookup[q];
        if (a<0) a=0;
        if (b>0) TV.draw_line (a,screeny,b,screeny,col);
        c = x2lookup[q];
        d = x2lookup[q]-xbarlookup[q];
        if (c>viewWidth) c=viewWidth;
        if (d<viewWidth) TV.draw_line (c,screeny,d,screeny,col);
    }



    if (Controller.upPressed()) zspeed+=1;
    if (Controller.downPressed()) zspeed-=1;
    if (Controller.leftPressed()) roadw+=10;
    if (Controller.rightPressed()) roadw-=10;
    z_car += zspeed/10 ;
    z_spots += zspeed/20 ;
    if (z_car>800) z_car=200;

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

