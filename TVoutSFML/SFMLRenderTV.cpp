#include <SFML/Graphics.hpp>
#include "wrapper.h"
#include "TVout.h"
#include "game.h"

using namespace std;
using namespace sf;

TVout TV;
sf::Texture TVtexture;
sf::Sprite TVsprite;
unsigned int windowWidth=800, windowHeight=600;
float viewZoom = 1.f;
sf::RectangleShape myPixel;
RenderWindow window{{windowWidth, windowHeight}, "SFML TVout"};
sf::View tv(sf::FloatRect(0, 0, TVX,TVY));
sf::Event event;

int pollEvent()
{
window.pollEvent(event);
    if(event.type == sf::Event::Closed || Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
       window.close();
    return -1;
    }
return 0;
}

int pollFire()
{
window.pollEvent(event);
    if(Keyboard::isKeyPressed(Keyboard::Key::Space)) return 1;
    return 0;
}
int pollLeft()
{
window.pollEvent(event);
    if(Keyboard::isKeyPressed(Keyboard::Key::Left)) return 1;
    return 0;
}
int pollRight()
{
window.pollEvent(event);
    if(Keyboard::isKeyPressed(Keyboard::Key::Right)) return 1;
    return 0;
}
int pollUp()
{
window.pollEvent(event);
    if(Keyboard::isKeyPressed(Keyboard::Key::Up)) return 1;
    return 0;
}
int pollDown()
{
window.pollEvent(event);
    if(Keyboard::isKeyPressed(Keyboard::Key::Down)) return 1;
    return 0;
}


void TVsetup()
{
    window.setFramerateLimit(60);
    myPixel.setSize(sf::Vector2f(1,1));
    myPixel.setFillColor(sf::Color::White);
    tv.zoom(viewZoom);
    tv.setViewport(sf::FloatRect(0, 0, 1, 1));
    window.clear(Color::Black);
    window.setView(tv);
    //sf::Thread update(&updateTV,&window,&tv);
}

void refresh(){
    window.setView(tv);
    window.clear(Color::Black);
    for (int y=0; y < TVY; y++) {
        for (int x=0; x < TVX; x ++) {
            if (display.screen[x/8+y*display.hres] & (0x80 >>(x&7)))
                {
                    myPixel.setPosition(x,y);
                    window.draw(myPixel);
                }
            }
        }
    window.display();
    pollEvent();
}
