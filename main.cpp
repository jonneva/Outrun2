// Outrunduino -Jonne Valola 2014

#include <SFML/Graphics.hpp>
#include "wrapper.h"
#include "TVout.h"
#include "font4x6.h"
#include "game.h"
#include "ferrari.h"
#include "sky.h"
#include <Controllers.h>

using namespace sf;

// START OF ARDUINO CODE

void updatecar();
void updateHUD();

void setup() {
    TV.begin(_PAL, TVX, TVY);
    TV.select_font(font4x6);
    gameSetup();
}

void loop() {
    TV.delay_frame(10);
    TV.draw_rect(0,0,TVX,dynamichz,0,0);
    if (track1[segment]&CURVELEFT) skyx+= zspeed/100;
    if (track1[segment]&CURVERIGHT) skyx-= zspeed/100;
    skyy=dynamichz-12;
    if (skyx>=TVX) skyx=0;
    if (skyx<0) skyx = TVX+skyx;
    //skyy =-1;
    drawSky(skyx,skyy);
    //updateHUD();
    if (track1[segment+2]==0xff) segment = 0;
    drawRoad();
    updatecar();

}

void updateHUD(){
    //TV.draw_line(0,HORIZON+1,TVX,HORIZON+1,1);
    TV.print(TVXCENTER+20,1,"      ");
    //TV.print(0,1,z_world);
    TV.print(0,0,"       ");
    //TV.print(0,8,dynamichz);
    if (hillstate==0) TV.print(0,0,"FLAT");
    if (hillstate==1) TV.print(0,0,"HILLC");
    if (hillstate==2) TV.print(0,0,"UP");
    if (hillstate==3) TV.print(0,0,"CREST");
    if (hillstate==4) TV.print(0,0,"DROP");
    if (hillstate==5) TV.print(0,0,"DROP2");
    if (hillstate==6) TV.print(0,0,"DOWN");
    if (hillstate==7) TV.print(0,0,"VALLEY");
    if (zspeed < 10) TV.print(TVXCENTER+20+3*6,1,zspeed);
    else if (zspeed < 100) TV.print(TVXCENTER+20+2*6,1,zspeed);
    if (zspeed > 100) TV.print(TVXCENTER+20+1*6,1,zspeed);

}

void updatecar() {
    // draw car
    byte nothingpressed = true;
    if (car_dir == UP) { overlaybitmap(carx,cary, ferrari_fwd, 0, 0, 0); TV.set_pixel(carx+13,cary+15,1); TV.set_pixel(carx+21,cary+15,1);}
    if (car_dir == LEFT) { overlaybitmap(carx,cary, ferrari_left, 0, 0, 0); TV.set_pixel(carx+21,cary+15,1); TV.set_pixel(carx+20,cary+15,1);TV.set_pixel(carx+13,cary+15,1); TV.set_pixel(carx,cary+10,1);TV.set_pixel(carx,cary+11,1); }
    if (car_dir == RIGHT) { overlaybitmap(carx,cary, ferrari_right, 0, 0, 0); TV.set_pixel (carx+13,cary+15,1); TV.set_pixel(carx+14,cary+15,1); TV.set_pixel(carx+20,cary+15,1); TV.set_pixel(carx+34,cary+10,1);TV.set_pixel(carx+34,cary+11,1); }
    car_dir = UP; // face forward unless left or right is pressed
    wheeloffset = 0;

    if (Controller.upPressed()) {
            zspeed+=10;
            if (acceltick==0) acceltick = ACCELTIME;
            nothingpressed = false;
    }

    if (Controller.downPressed()) {
        zspeed-=15;
        nothingpressed = false;
        fumeframe = 9;
        acceltick = 1;
    }
    if (zspeed<0) zspeed = 0;
    if (zspeed>293) zspeed = 293;
    if (Controller.leftPressed()) {
            roadx+=(1+zspeed/100);
            car_dir = LEFT;
            wheeloffset = -1;
            if (zspeed>100 && acceltick==0) acceltick = ACCELTIME/4;
            nothingpressed = false;
            }
    if (Controller.rightPressed()) {
            roadx-=(1+zspeed/100);
            car_dir = RIGHT;
            wheeloffset = 1;
            if (zspeed>100 && acceltick==0) acceltick = ACCELTIME/4;
            nothingpressed = false;
    }

    if (acceltick>0) {
                if (fumeframe == 0) overlaybitmap(carx-13+wheeloffset,cary+14,fumes1,0,0,0);
                if (fumeframe == 3) overlaybitmap(carx-13+wheeloffset,cary+14,fumes2,0,0,0);
                if (fumeframe == 9) overlaybitmap(carx-13+wheeloffset,cary+14,fumes3,0,0,0);
                fumeframe++;
                if (fumeframe == 12) fumeframe=0;
                acceltick--;
    }


     if (deceltick) zspeed-=2; deceltick = !deceltick;
     if (nothingpressed) {acceltick = 0;fumeframe=0;}

    z_car += zspeed/10 ;
    //z_world += zspeed/5 ;
    z_world += zspeed/8;
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

int getMemory() {
  int size = 512;
  byte *buf;
  while ((buf = (byte *) malloc(--size)) == NULL);
  free(buf);
  return size;
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

