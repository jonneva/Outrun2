#include "TVout.h"
#include "wrapper.h"

// Bitmap overlay functions

void overlaybitmap(uint8_t x, uint8_t y, const unsigned char * bmp,
				   uint16_t i, uint8_t width, uint8_t lines) {

	uint8_t temp, lshift, rshift, save, xtra;
	uint16_t si = 0;

	rshift = x&7;
	lshift = 8-rshift;
	if (width == 0) {
	  //width = pgm_read_byte((uint32_t)(bmp) + i);
	  width = bmp[i];
	  i++;
	}
	if (lines == 0) {
	  //lines = pgm_read_byte((uint32_t)(bmp) + i);
	  lines = bmp[i];
	  i++;
	}

	if (width&7) {
	  xtra = width&7;
	  width = width/8;
	  width++;
	}
	else {
	  xtra = 8;
	  width = width/8;
	}

	for (uint8_t l = 0; l < lines; l++) {
	  si = ((y + l) % display.vres)*display.hres + x/8;
	  //si = (y + l)*display.hres + x/8;
	  if (width == 1)
	    temp = 0xff >> rshift + xtra;
	  else
	    temp = 0;
	  save = display.screen[si];
	  //temp = pgm_read_byte((uint32_t)(bmp) + i++);
	  temp = bmp[i++];
	  display.screen[si++] |= temp >> rshift;
	  for ( uint16_t b = i + width-1; i < b; i++) {
	    if (si % display.hres == 0) {
	      // wrapped around to the left side
	      si -= display.hres;
	    }
	    save = display.screen[si];
	    display.screen[si] |= temp << lshift;
	    //temp = pgm_read_byte((uint32_t)(bmp) + i);
	    temp = bmp[i];
	    display.screen[si++] |= temp >> rshift;
	  }
	  if (si % display.hres == 0) {
	    // wrapped around to the left side
	    si -= display.hres;
	  }
	  if (rshift + xtra < 8)
	    display.screen[si-1] |= (save & (0xff >> rshift + xtra));	//test me!!!
	  display.screen[si] |= temp << lshift;
	}
} // end of bitmap

void erasebitmap(uint8_t x, uint8_t y, const unsigned char * bmp,
				   uint16_t i, uint8_t width, uint8_t lines) {

	uint8_t temp, lshift, rshift, save, xtra;
	uint16_t si = 0;

	rshift = x&7;
	lshift = 8-rshift;
	if (width == 0) {
		//width = pgm_read_byte((uint32_t)(bmp) + i);
		width = bmp[i];
		i++;
	}
	if (lines == 0) {
		//lines = pgm_read_byte((uint32_t)(bmp) + i);
		lines = bmp[i];
		i++;
	}

	if (width&7) {
		xtra = width&7;
		width = width/8;
		width++;
	}
	else {
		xtra = 8;
		width = width/8;
	}

	for (uint8_t l = 0; l < lines; l++) {
	  si = ((y + l) % display.vres)*display.hres + x/8;
	  //si = (y + l)*display.hres + x/8;
		if (width == 1)
			temp = 0xff >> rshift + xtra;
		else
			temp = 0;
		save = display.screen[si];
		//temp = pgm_read_byte((uint32_t)(bmp) + i++);
		temp = bmp[i++];
		display.screen[si++] &= ~(temp >> rshift);
		for ( uint16_t b = i + width-1; i < b; i++) {
		  if (si % display.hres == 0) {
		    // wrapped around to the left side
		    si -= display.hres;
		  }
			save = display.screen[si];
			display.screen[si] &= ~(temp << lshift);
			//temp = pgm_read_byte((uint32_t)(bmp) + i);
			temp = bmp[i];
			display.screen[si++] &= ~(temp >> rshift);
		}
		if (si % display.hres == 0) {
		  // wrapped around to the left side
		  si -= display.hres;
		}

		if (rshift + xtra < 8)
			display.screen[si-1] &= ~(save & (0xff >> rshift + xtra));	//test me!!!
		if (rshift + xtra - 8 > 0)
		  display.screen[si] &= ~(temp << lshift);
	}
} // end of bitmap

void alphabitmap(uint8_t x, uint8_t y, const unsigned char * bmp, const unsigned char * alpha) {

    // NEEDS AT LEAST 1 byte wide bitmap !!!! Alpha needs to be same size !
	uint8_t temp, lshift, rshift, save, xtra, width, lines;
	uint8_t atemp, maskedbyte, onbits, offbits, outputbyte, xoredbyte, andedbyte;
	uint16_t i=0, si = 0;
    //x=63;
	rshift = x&7;      // rshift = bits OVER even bytes, eg. 14 = 110
	lshift = 8-rshift; // lshift = bits UNTIL next even byte, eg. 2 = 10

    TV.print(0,30,"rshift: ");
    TV.print(rshift);

	// get image buffer size
	width = bmp[i]; i++; //width = pgm_read_byte((uint32_t)(bmp) + i);
	lines = bmp[i]; i++;//lines = pgm_read_byte((uint32_t)(bmp) + i);

    // check if image buffer is divisible by 8
	if (width&7) {      // if width is not divisible by 8
	  xtra = width&7;   // xtra is amount of xtra bits
	  width = width/8;  // width is width in even bytes
	  width++;          // plus one uneven one ...
	}
	else {              // width IS divisible by 8
	  xtra = 8;         // xtra = 1000b
	  width = width/8;  // width = even num of bytes
	}

    // start drawing routine, iterate through 0 to num of lines
	for (uint8_t l = 0; l < lines; l++) {
	  si = ((y + l) % display.vres)*display.hres + x/8; // si is index to screen

	  save = display.screen[si]; // Save is the screen byte
	  atemp = alpha[i]; // read first byte of alpha data
	  temp = bmp[i]; i++; // temp = pgm_read_byte((uint32_t)(bmp) + i++); //bmp data

      onbits = temp & atemp; // get bits that should be turned on
      offbits = ~temp & atemp; // get bits that should be turned off
      temp = (onbits >> rshift ) | save; // add onbits
      temp = temp & (~(offbits >> rshift)); // delete offbits

      display.screen[si] = temp; si++; //first byte to screen buffer, increment

	  // start looping through whole bytes
	  for ( uint16_t b = i + width-1; i < b; i++) {
	    if (si % display.hres == 0) {
	      //si -= display.hres; // wrap disabled
	    } else {
            // draw the leftovers of a previous shifted byte first
            save = display.screen[si]; // get byte at si
            temp = (onbits << lshift ) | save; // add onbits
            temp = temp & (~(offbits << lshift)); // delete offbits
            save = temp; // waiting for ls bits of the next bmp byte
            // add the leftside bits of a new image byte
            atemp = alpha[i]; // read byte of alpha data
            temp = bmp[i];
            onbits = temp & atemp; // get bits that should be turned on
            offbits = ~temp & atemp; // get bits that should be turned off
            temp = (onbits >> rshift) | save; // add onbits
            temp = temp & (~(offbits >> rshift)); // delete offbits
            display.screen[si] = temp; // was temp
            si++; // increment screen index to next screen byte
	    }
	  } // END OF X LOOP i.e. DRAW 1 ROW OF BITMAP TO SCREEN !!

	  if (si % display.hres == 0) { // now drawing last byte of row, check for edge
	    //si -= display.hres; // I need to disable this
	  } else {
        save = display.screen[si];  // get next screen byte
        temp = (onbits << lshift ) | save; // add leftover onbits
        temp = temp & (~(offbits << lshift)); // delete leftover
        display.screen[si] = temp;// save; //temp; // store 8-lshift leftmost bits
	  }

	//TV.delay(1000);
	} // END OF LINES (Y) LOOP !!!

    //TV.draw_rect(60,5,10,10,0,0);
} // end of bitmap

