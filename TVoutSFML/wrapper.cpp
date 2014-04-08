#include <SFML/Graphics.hpp>
#include "wrapper.h"

int random(int a, int b) {
    int r; //Arduino random(a,b) EXCLUDES b !!!
    if (b < 1) return 0;
    if (a < 0) return 0;
    r = rand() % (b - a) + a ;
    return  r;
}

byte pgm_read_byte(unsigned char* pointer) {
    return *pointer;
}

byte pgm_read_word(unsigned int* pointer) {
    return *pointer;
}
