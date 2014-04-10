#ifndef WRAPPER_H
#define WRAPPER_H
#include <SFML/Graphics.hpp>
#include "TVout.h"

typedef unsigned char byte;
typedef unsigned char prog_char;
typedef bool boolean;
#define PROGMEM

extern int random(int, int);
extern byte pgm_read_byte(unsigned char*);
extern byte pgm_read_word(unsigned int*);

extern void overlaybitmap(int16_t, int16_t, const unsigned char*,
				   uint16_t, uint8_t, uint8_t);
extern void erasebitmap(uint8_t, uint8_t, const unsigned char*,
				   uint16_t, uint8_t, uint8_t);
extern void alphabitmap(uint8_t, uint8_t, const unsigned char*,const unsigned char*);

extern void refresh();
extern void TVsetup();
extern int pollEvent();
extern int pollFire();
extern int pollLeft();
extern int pollRight();
extern int pollUp();
extern int pollDown();

extern TVout TV;
extern sf::Texture TVtexture;
extern sf::Sprite TVsprite;
extern unsigned int windowWidth, windowHeight, viewWidth, viewHeight;
extern float viewZoom;
extern sf::RectangleShape myPixel;
extern sf::RenderWindow window;
extern sf::View tv;
extern sf::Event event;
extern bool keyLeft, keyRight, keyUp, keyDown, keyFire;

#define PI 3.14159265
#endif
