// Outrunduino -Jonne Valola 2014

#include <SFML/Graphics.hpp>
//#include <iostream>
//#include <math.h>
#include "wrapper.h"
#include "TVout.h"
#include "fontALL.h"
#include "game.h"
#include "ferrari.h"
#include "sky.h"

//using namespace std;
using namespace sf;

// START OF ARDUINO CODE
int skyy = PARALLAXTOP; int turn=-5;
#define SKY

#include <Controllers.h>

void setup() {
    int a=6;
    TV.begin(_PAL, TVX, TVY);
    TV.select_font(font4x6);
    gameSetup();
    #ifdef SKY
    drawSky(skyx-turn,skyy-2);
    #endif // SKY
}

void loop() {
    int xtemp,zlook;

    TV.delay_frame(10);
    #ifdef SKY
    TV.draw_rect(0,0,TVX,PARALLAXTOP,0,0);
    drawSky(skyx-turn,skyy-2);
    if (skytick > 1){
        turn +=zspeed/150;
        //TV.shift(turn,LEFT);
    }
    #endif
    TV.draw_line(0,HORIZON+1,TVX,HORIZON+1,1);
    TV.print(TVXCENTER+20,1,"      ");
    TV.print(0,1,z_spots);
    if (zspeed < 10) TV.print(TVXCENTER+20+3*6,1,zspeed);
    else if (zspeed < 100) TV.print(TVXCENTER+20+2*6,1,zspeed);
    if (zspeed > 100) TV.print(TVXCENTER+20+1*6,1,zspeed);
    threeLanes();
    //oneLane();

    // draw car
    if (car_dir == UP) { overlaybitmap(carx,cary, ferrari_fwd, 0, 0, 0); TV.set_pixel(carx+13,cary+15,1); TV.set_pixel(carx+21,cary+15,1);}
    if (car_dir == LEFT) { overlaybitmap(carx,cary, ferrari_left, 0, 0, 0); TV.set_pixel(carx+21,cary+15,1); TV.set_pixel(carx+20,cary+15,1);TV.set_pixel(carx+13,cary+15,1); TV.set_pixel(carx,cary+10,1);TV.set_pixel(carx,cary+11,1); }
    if (car_dir == RIGHT) { overlaybitmap(carx,cary, ferrari_right, 0, 0, 0); TV.set_pixel (carx+13,cary+15,1); TV.set_pixel(carx+14,cary+15,1); TV.set_pixel(carx+20,cary+15,1); TV.set_pixel(carx+34,cary+10,1);TV.set_pixel(carx+34,cary+11,1); }
    car_dir = UP; // face forward unless left or right is pressed
    wheeloffset = 0;
    if (Controller.upPressed()) {
            if (acceltick>0) {
                if (fumeframe == 0) overlaybitmap(carx-13+wheeloffset,cary+14,fumes1,0,0,0);
                if (fumeframe == 3) overlaybitmap(carx-13+wheeloffset,cary+14,fumes2,0,0,0);
                if (fumeframe == 9) overlaybitmap(carx-13+wheeloffset,cary+14,fumes3,0,0,0);
                fumeframe++;
                if (fumeframe == 12) fumeframe=0;
                acceltick--;
            }
            zspeed+=10;
    } else {
        if (deceltick) zspeed-=4;
        deceltick = !deceltick;
        if (acceltick != ACCELTIME ) {
        acceltick = ACCELTIME;
        fumeframe=0;
        }
    }
    if (Controller.downPressed()) zspeed-=15;
    if (zspeed<0) zspeed = 0;
    if (zspeed>293) zspeed = 293;
    if (Controller.leftPressed()) {
            roadx+=(1+zspeed/100);
            car_dir = LEFT;
            wheeloffset = -1;
            }
    if (Controller.rightPressed()) {
            roadx-=(1+zspeed/100);
            car_dir = RIGHT;
            wheeloffset = 1;
    }
    z_car += zspeed/10 ;
    //z_spots += zspeed/5 ;
    z_spots += zspeed/8;
    skytick+= zspeed/100;
    if (z_car>100) {
            z_car=0;
    }
    if (zspeed>10) {
            if (wheeltick == 0) TV.bitmap(carx+1+wheeloffset, cary+17, wheels1);
            if (wheeltick == 1) TV.bitmap(carx+1+wheeloffset, cary+17, wheels2);
            if (wheeltick == 2) {
                    TV.bitmap(carx+1+wheeloffset, cary+17, wheels3);
                    wheeltick = 0;
            } else { wheeltick++; }
    }
    if (zspeed>200) { // hair starts blowing !
            if (wheeltick == 0) {TV.set_pixel(carx+24+wheeloffset+wheeloffset,cary+3,1);TV.set_pixel(carx+24+wheeloffset+wheeloffset,cary+4,0);}
            if (wheeltick == 2) {TV.set_pixel(carx+19+wheeloffset+wheeloffset,cary+3,1);TV.set_pixel(carx+19+wheeloffset+wheeloffset,cary+4,0);}
    }
}


// END OF ARDUINO CODE

int main()
{
    //std::cout << "SFML TVout emulator started" << std::endl;
	TVsetup();
    setup();

while (window.isOpen())
    {
	loop();
    }
	return 0;
}

