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
	uint8_t atemp, maskedbyte, outputbyte, xoredbyte, andedbyte;
	uint16_t i=0, si = 0;

    // i is used for data including multiple bitmaps

    // define if screen pointer needs to be bitshifted
	rshift = x&7;      // rshift = bits OVER even bytes, eg. 14 = 110
	lshift = 8-rshift; // lshift = bits UNTIL next even byte, eg. 2 = 10

	// get image buffer size
	//width = pgm_read_byte((uint32_t)(bmp) + i);
	width = bmp[i];
	i++;
	//lines = pgm_read_byte((uint32_t)(bmp) + i);
	lines = bmp[i];
	i++;

    // check if image buffer is divisible by 8
	if (width&7) {      // if width is not divisible by 8
	  xtra = width&7;   // xtra is amount of xtra bits
	  width = width/8;  // width is width in even bytes
	  width++;          // plus one uneven one ...
	}
	else {              // width IS divisble by 8
	  xtra = 8;         // xtra = 1000b
	  width = width/8;  // width = even num of bytes
	}

    // start drawing routine
    // iterate through 0 to num of lines
	for (uint8_t l = 0; l < lines; l++) {
      // si is index to screen, eg. 10,50 is (51 % 56)*104 + 10/8 = byte no. 5305
	  si = ((y + l) % display.vres)*display.hres + x/8;
      // if image width is only 1 byte, for example 5 bits
      // width would have been calculated as 1
      // rshift = screen pointer shift !!! for example point 6,6 rshift = 110
      // xtra = is image width bits above even bytes, in this case 5
	  if (width == 1)
        //temp = (1111 1111 >> 6) + 101 = 11 + 101 = 8 = 1000
	    temp = 0xff >> rshift + xtra; // temp gets overwritten later !! doesnt work !!
	  else
	    temp = 0; // image is wider than 1 byte, no need to store byte separately

      // store byte at si in screen buf
	  save = display.screen[si]; // Save is the screen byte

	  //temp = pgm_read_byte((uint32_t)(bmp) + i++);

	  // point temp to first image data byte
	  atemp = alpha[i]; // read first byte of alpha data
	  temp = bmp[i++]; // read first byte of image data, increment after read

      // to make alpha work, need to combine screen, bmp and alpha so that:
      // ((BMP && MASK) ^ SCREEN) OR ((BMP && MASK) && SCREEN)

      maskedbyte = (temp & atemp); //>> rshift; // BMP && MASK, shifted to landing
      xoredbyte = maskedbyte ^ save;
      andedbyte = ~save;
      outputbyte = xoredbyte & andedbyte;

	  // put first byte into buffer, increment si after write
	  // temp (the byte from image) is shifted right by for example 5 for (5,0)
	  //display.screen[si++] ^= ~(temp >> rshift); //first byte added to buffer
      display.screen[si++] = outputbyte; //this gets drawn properly

	  // start looping through the rest of the bytes
	  // increment from b = second byte + width of IMAGE row - 1
	  // to until i == b == end of 1 row within source image
	  for ( uint16_t b = i + width-1; i < b; i++) {
	    if (si % display.hres == 0) {
          //if si modulus hres is zero were at r edge of screen buffer
	      // wrapped around to the left side
	      //si -= display.hres; // I need to disable this
	    } else {
            save = display.screen[si]; // save byte at si
            maskedbyte = (temp & atemp) << lshift; // BMP && MASK, shifted to landing
            outputbyte = (maskedbyte ^ save) & (maskedbyte & save);
            // lshift is needed if bitmap doesn't land on screen in even bytes
            // display.screen[si] ^= ~(temp << lshift); // store 8-lshift leftmost bits
            display.screen[si] = outputbyte; // store 8-lshift leftmost bits
            //temp = pgm_read_byte((uint32_t)(bmp) + i);
            atemp = alpha[i]; // load new byte of alpha data
            temp = bmp[i]; // load new byte from image buffer to temp
            // display.screen[si++] ^= ~(temp >> rshift); // store rside bytes to screen...
            // same procedure as with first byte done before
            maskedbyte = (temp & atemp) >> rshift; // BMP && MASK, shifted to landing
            outputbyte = (maskedbyte ^ save) & (maskedbyte & save);
            display.screen[si++] = outputbyte;
	    }
	  } // END OF X LOOP i.e. DRAW 1 ROW OF BITMAP TO SCREEN !!

	  if (si % display.hres == 0) { // now drawing last byte of row, check for edge
	    // wrapped around to the left side
	    //si -= display.hres; // I need to disable this
	  } else {
	     save = display.screen[si];  // this was missing
	    // last byte, no wraparound
	    // for example for (5,0) rshift = 5
	    // and for 15 bits wide image, xtra is 111 = 7
            if (rshift + xtra < 8) // if its a less than 8-bit wide image... nonsense
            display.screen[si-1] ^= ~(save & (0xff >> rshift + xtra));	//test me!!!

        // the real deal is this ofcourse. Last byte of ROW written
        // display.screen[si] ^= ~(temp << lshift); // write the leftmost bits if image
        maskedbyte = (temp & atemp) << lshift; // BMP && MASK, shifted to landing
        outputbyte = (maskedbyte ^ save) & (maskedbyte & save);
        //outputbyte = (maskedbyte ^ (save << lshift)) | (maskedbyte & (save << lshift));
        display.screen[si] = outputbyte; // store 8-lshift leftmost bits
	  }
	TV.delay(1000);
	} // END OF LINES (Y) LOOP !!!

    //TV.draw_rect(60,5,10,10,0,0);
} // end of bitmap

