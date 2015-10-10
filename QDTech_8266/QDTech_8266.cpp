/***************************************************
  This is a library for the Adafruit 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "QDTech_8266.h"
#include <limits.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>

#if !defined(ST7735_USE_HWSPI_ONLY)
// Constructor when using software SPI.  All output pins are configurable.
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t rs, int8_t sid, int8_t sclk, int8_t rst) 
  : Adafruit_GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT_18)
{
  _cs   = cs;
  _rs   = rs;
  _sid  = sid;
  _sclk = sclk;
  _rst  = rst;
  hwSPI = false;
}
#endif

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t rs, int8_t rst) 
  : Adafruit_GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT_18) {
  _cs   = cs;
  _rs   = rs;
  _rst  = rst;
  hwSPI = true;
  _sid  = _sclk = 0;
}

#if defined(CORE_TEENSY) && !defined(__AVR__)
#define __AVR__
#endif

#if defined(ST7735_USE_HWSPI_ONLY)
inline void Adafruit_ST7735::spiwrite(uint8_t c) {
  SPI.transfer(c);
}
#elif defined(__AVR__)
inline void Adafruit_ST7735::spiwrite(uint8_t c) {

  //Serial.println(c, HEX);

  if (hwSPI) {
    SPDR = c;
    while(!(SPSR & _BV(SPIF)));
  } else {
    // Fast SPI bitbang swiped from LPD8806 library
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
      if(c & bit) *dataport |=  datapinmask;
      else        *dataport &= ~datapinmask;
      *clkport |=  clkpinmask;
      *clkport &= ~clkpinmask;
    }
  }
}
#elif defined(__SAM3X8E__)
inline void Adafruit_ST7735::spiwrite(uint8_t c) {
  
  //Serial.println(c, HEX);
  
  if (hwSPI) {
    SPI.transfer(c);
  } else {
    // Fast SPI bitbang swiped from LPD8806 library
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
      if(c & bit) dataport->PIO_SODR |= datapinmask;
      else        dataport->PIO_CODR |= datapinmask;
      clkport->PIO_SODR |= clkpinmask;
      clkport->PIO_CODR |= clkpinmask;
    }
  }
}
#else
#error Unsupported board.
#endif

#if defined(ST7735_USE_HWSPI_WRITE16)
inline void Adafruit_ST7735::spiwrite16(uint16_t v) {
  SPI.write16(v);
}
#else
inline void Adafruit_ST7735::spiwrite16(uint16_t v) {
  spiwrite((uint8_t)(v >> 8));
  spiwrite((uint8_t)(v >> 0));
}
#endif

#if defined(ST7735_USE_GENERIC_IO)
inline void Adafruit_ST7735::setCS(bool level) {
  digitalWrite(_cs, level);
}

inline void Adafruit_ST7735::setRS(bool level) {
  digitalWrite(_rs, level);
}
#elif defined(__AVR__)
inline void Adafruit_ST7735::setCS(bool level) {
  if (level) {
    *csport |= cspinmask;
  } else {
    *csport &= ~cspinmask;
  }
}

inline void Adafruit_ST7735::setRS(bool level) {
  if (level) {
    *rsport |= rspinmask;
  } else {
    *rsport &= ~rspinmask;
  }
}
#elif defined(__SAM3X8E__)
inline void Adafruit_ST7735::setCS(bool level) {
  if (level) {
    csport->PIO_SODR |=  cspinmask;
  } else {
    csport->PIO_CODR |=  cspinmask;
  }
}

inline void Adafruit_ST7735::setRS(bool level) {
  if (level) {
    rsport->PIO_SODR |=  rspinmask;
  } else {
    rsport->PIO_CODR |=  rspinmask;
  }
}
#else
#error Unsupported board.
#endif

void Adafruit_ST7735::writecommand(uint8_t c) {
  setRS(false);
  setCS(false);

  //Serial.print("C ");
  spiwrite(c);

  setCS(true);
}


void Adafruit_ST7735::writedata(uint8_t c) {
  setRS(true);
  setCS(false);
    
  //Serial.print("D ");
  spiwrite(c);

  setCS(true);
} 

void Adafruit_ST7735::writedata16(uint16_t c) {
  setRS(true);
  setCS(false);
    
  //Serial.print("D ");
  spiwrite16(c);

  setCS(true);
} 

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80
static const uint8_t PROGMEM	// Multiple LCD init commands removed
  QDTech[] = {                  // QDTech support only now
	29,
	0xf0,	2,	0x5a, 0x5a,				// Excommand2
	0xfc,	2,	0x5a, 0x5a,				// Excommand3
	0x26,	1,	0x01,					// Gamma set
	0xfa,	15,	0x02, 0x1f,	0x00, 0x10,	0x22, 0x30, 0x38, 0x3A, 0x3A, 0x3A,	0x3A, 0x3A,	0x3d, 0x02, 0x01,	// Positive gamma control
	0xfb,	15,	0x21, 0x00,	0x02, 0x04,	0x07, 0x0a, 0x0b, 0x0c, 0x0c, 0x16,	0x1e, 0x30,	0x3f, 0x01, 0x02,	// Negative gamma control
	0xfd,	11,	0x00, 0x00, 0x00, 0x17, 0x10, 0x00, 0x01, 0x01, 0x00, 0x1f, 0x1f,							// Analog parameter control
	0xf4,	15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x07, 0x00, 0x3C, 0x36, 0x00, 0x3C, 0x36, 0x00,	// Power control
	0xf5,	13, 0x00, 0x70, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6d, 0x66, 0x06,				// VCOM control
	0xf6,	11, 0x02, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x02, 0x00, 0x06, 0x01, 0x00,							// Source control
	0xf2,	17, 0x00, 0x01, 0x03, 0x08, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x04, 0x08, 0x08,	//Display control
	0xf8,	1,	0x11,					// Gate control
	0xf7,	4, 0xc8, 0x20, 0x00, 0x00,	// Interface control
	0xf3,	2, 0x00, 0x00,				// Power sequence control
	0x11,	DELAY, 50,					// Wake
	0xf3,	2+DELAY, 0x00, 0x01, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0x03, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0x07, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0x0f, 50,	// Power sequence control
	0xf4,	15+DELAY, 0x00, 0x04, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x07, 0x00, 0x3C, 0x36, 0x00, 0x3C, 0x36, 0x00, 50,	// Power control
	0xf3,	2+DELAY, 0x00, 0x1f, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0x7f, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0xff, 50,	// Power sequence control
	0xfd,	11, 0x00, 0x00, 0x00, 0x17, 0x10, 0x00, 0x00, 0x01, 0x00, 0x16, 0x16,							// Analog parameter control
	0xf4,	15, 0x00, 0x09, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x07, 0x00, 0x3C, 0x36, 0x00, 0x3C, 0x36, 0x00,	// Power control
	0x36,	1, 0x08,					// Memory access data control
	0x35,	1, 0x00,					// Tearing effect line on
	0x3a,	1+DELAY, 0x05, 150,			// Interface pixel control
	0x29,	0,							// Display on
	0x2c,	0							// Memory write
  };


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Adafruit_ST7735::commandList(const uint8_t *addr) {

  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  
  while(numCommands--) {                 // For each command...
	writecommand(pgm_read_byte(addr++)); //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
    numArgs &= ~DELAY;                   //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata(pgm_read_byte(addr++));  //     Read, issue argument
    }
    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
}


// Initialization code common to both 'B' and 'R' type displays
void Adafruit_ST7735::commonInit(const uint8_t *cmdList) {
  colstart  = rowstart = 0; // May be overridden in init func

  pinMode(_rs, OUTPUT);
  pinMode(_cs, OUTPUT);
#ifdef __AVR__
  csport    = portOutputRegister(digitalPinToPort(_cs));
  rsport    = portOutputRegister(digitalPinToPort(_rs));
#endif
#if defined(__SAM3X8E__)
  csport    = digitalPinToPort(_cs);
  rsport    = digitalPinToPort(_rs);
#endif
#if !defined(ST7735_USE_GENERIC_IO)  
  cspinmask = digitalPinToBitMask(_cs);
  rspinmask = digitalPinToBitMask(_rs);
#endif
  if(hwSPI) { // Using hardware SPI
    SPI.begin();
#ifdef __AVR__
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz (half speed)
    //Due defaults to 4mHz (clock divider setting of 21)
#elif defined(__SAM3X8E__)
    SPI.setClockDivider(21); // 4 MHz
    //Due defaults to 4mHz (clock divider setting of 21), but we'll set it anyway 
#else
    // Use 8MHz for other boards.
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz
#endif
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
  } else {
    pinMode(_sclk, OUTPUT);
    pinMode(_sid , OUTPUT);
#ifdef __AVR__
    clkport     = portOutputRegister(digitalPinToPort(_sclk));
    dataport    = portOutputRegister(digitalPinToPort(_sid));
#endif
#if defined(__SAM3X8E__)
    clkport     = digitalPinToPort(_sclk);
    dataport    = digitalPinToPort(_sid);
#endif
#if !defined(ST7735_USE_GENERIC_IO)
    clkpinmask  = digitalPinToBitMask(_sclk);
    datapinmask = digitalPinToBitMask(_sid);
#endif
#ifdef __AVR__
    *clkport   &= ~clkpinmask;
    *dataport  &= ~datapinmask;
#endif
#if defined(__SAM3X8E__)
    clkport ->PIO_CODR  |=  clkpinmask; // Set control bits to LOW (idle)
    dataport->PIO_CODR  |=  datapinmask; // Signals are ACTIVE HIGH
#endif
  }

  // toggle RST low to reset; CS low so it'll listen to us
  setCS(false);
  if (_rst) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(500);
    digitalWrite(_rst, LOW);
    delay(500);
    digitalWrite(_rst, HIGH);
    delay(500);
  }

  if(cmdList) commandList(cmdList);
}


// Initialization for ST7735B screens
void Adafruit_ST7735::init() {
  commonInit(QDTech);
}


/* Initialization for ST7735R screens (green or red tabs)
void Adafruit_ST7735::initR(uint8_t options) {
  commonInit(Rcmd1);
  if(options == INITR_GREENTAB) {
    commandList(Rcmd2green);
    colstart = 2;
    rowstart = 1;
  } else if(options == INITR_144GREENTAB) {
    _height = ST7735_TFTHEIGHT_144;
    commandList(Rcmd2green144);
    colstart = 2;
    rowstart = 3;
  } else {
    // colstart, rowstart left at default '0' values
    commandList(Rcmd2red);
  }
  commandList(Rcmd3);

  // if black, change MADCTL color filter
  if (options == INITR_BLACKTAB) {
    writecommand(ST7735_MADCTL);
    writedata(0xC0);
  }

  tabcolor = options;
}*/


void Adafruit_ST7735::setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1,
 uint8_t y1) {

  writecommand(ST7735_CASET); // Column addr set
  writedata16(x0+colstart);     // XSTART 
  writedata16(x1+colstart);     // XEND

  writecommand(ST7735_RASET); // Row addr set
  writedata16(y0+rowstart);     // YSTART
  writedata16(y1+rowstart);     // YEND

  writecommand(ST7735_RAMWR); // write to RAM
}


void Adafruit_ST7735::pushColor(uint16_t color) {
  setRS(true);
  setCS(false);
  
  spiwrite16(color);

  setCS(true);
}

void Adafruit_ST7735::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

  setAddrWindow(x,y,x+1,y+1);
  writedata16(color);
}

void Adafruit_ST7735::drawFastVLine(int16_t x, int16_t y, int16_t h,
 uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((y+h-1) >= _height) h = _height-y;
  setAddrWindow(x, y, x, y+h-1);
  writeColor(color, h);
}


void Adafruit_ST7735::drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  setAddrWindow(x, y, x+w-1, y);

  writeColor(color, w);
}



void Adafruit_ST7735::fillScreen(uint16_t color) {
  fillRect(0, 0,  _width, _height, color);
}


void Adafruit_ST7735::writeColor(uint16_t color, uint16_t count) {
  setRS(true);
  setCS(false);
#if defined(ST7735_USE_HWSPI_WRITEPATTERN)
  uint8_t pattern[] = { (uint8_t)(color >> 8), (uint8_t)(color >> 0) };
  SPI.writePattern(pattern, sizeof(pattern), count);
#else
  for (; count != 0; count--) {
    spiwrite16(color);
  }
#endif
  setCS(true);
}

// fill a rectangle
void Adafruit_ST7735::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  setAddrWindow(x, y, x+w-1, y+h-1);
  writeColor(color, w*h);
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ST7735::Color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void Adafruit_ST7735::setRotation(uint8_t m) {

  writecommand(ST7735_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     if (tabcolor == INITR_BLACKTAB) {
       writedata(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
     } else {
       writedata(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
     }
     _width  = ST7735_TFTWIDTH;

     if (tabcolor == INITR_144GREENTAB) 
       _height = ST7735_TFTHEIGHT_144;
     else
       _height = ST7735_TFTHEIGHT_18;

     break;
   case 1:
     if (tabcolor == INITR_BLACKTAB) {
       writedata(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
     } else {
       writedata(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
     }

     if (tabcolor == INITR_144GREENTAB) 
       _width = ST7735_TFTHEIGHT_144;
     else
       _width = ST7735_TFTHEIGHT_18;

     _height = ST7735_TFTWIDTH;
     break;
  case 2:
     if (tabcolor == INITR_BLACKTAB) {
       writedata(MADCTL_RGB);
     } else {
       writedata(MADCTL_BGR);
     }
     _width  = ST7735_TFTWIDTH;
     if (tabcolor == INITR_144GREENTAB) 
       _height = ST7735_TFTHEIGHT_144;
     else
       _height = ST7735_TFTHEIGHT_18;

    break;
   case 3:
     if (tabcolor == INITR_BLACKTAB) {
       writedata(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
     } else {
       writedata(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
     }
     if (tabcolor == INITR_144GREENTAB) 
       _width = ST7735_TFTHEIGHT_144;
     else
       _width = ST7735_TFTHEIGHT_18;

     _height = ST7735_TFTWIDTH;
     break;
  }
}


void Adafruit_ST7735::invertDisplay(boolean i) {
  writecommand(i ? ST7735_INVON : ST7735_INVOFF);
}


////////// stuff not actively being used, but kept for posterity
/*

 uint8_t Adafruit_ST7735::spiread(void) {
 uint8_t r = 0;
 if (_sid > 0) {
 r = shiftIn(_sid, _sclk, MSBFIRST);
 } else {
 //SID_DDR &= ~_BV(SID);
 //int8_t i;
 //for (i=7; i>=0; i--) {
 //  SCLK_PORT &= ~_BV(SCLK);
 //  r <<= 1;
 //  r |= (SID_PIN >> SID) & 0x1;
 //  SCLK_PORT |= _BV(SCLK);
 //}
 //SID_DDR |= _BV(SID);
 
 }
 return r;
 }
 
 
 void Adafruit_ST7735::dummyclock(void) {
 
 if (_sid > 0) {
 digitalWrite(_sclk, LOW);
 digitalWrite(_sclk, HIGH);
 } else {
 // SCLK_PORT &= ~_BV(SCLK);
 //SCLK_PORT |= _BV(SCLK);
 }
 }
 uint8_t Adafruit_ST7735::readdata(void) {
 *portOutputRegister(rsport) |= rspin;
 
 *portOutputRegister(csport) &= ~ cspin;
 
 uint8_t r = spiread();
 
 *portOutputRegister(csport) |= cspin;
 
 return r;
 
 } 
 
 uint8_t Adafruit_ST7735::readcommand8(uint8_t c) {
 digitalWrite(_rs, LOW);
 
 *portOutputRegister(csport) &= ~ cspin;
 
 spiwrite(c);
 
 digitalWrite(_rs, HIGH);
 pinMode(_sid, INPUT); // input!
 digitalWrite(_sid, LOW); // low
 spiread();
 uint8_t r = spiread();
 
 
 *portOutputRegister(csport) |= cspin;
 
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 
 uint16_t Adafruit_ST7735::readcommand16(uint8_t c) {
 digitalWrite(_rs, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 
 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 uint16_t r = spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 uint32_t Adafruit_ST7735::readcommand32(uint8_t c) {
 digitalWrite(_rs, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 
 dummyclock();
 dummyclock();
 
 uint32_t r = spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 */
