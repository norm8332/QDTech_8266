# QDTech Arduino ISP ESP-8266 1.8 inch TFT Library

  This is a modification of the Adafruit SPI TFT LCD Arduino 
  library, customised to run Directly on the ESP8266 using hardware (or software) SPI for 
  the QDTech 1.8" 128x160 pixel LCD board that uses a Samsung 
  S6D02A1 chip.
  
  Video of it in action: https://www.youtube.com/watch?v=vG-Dcr0BFBc
  
  It is for use with the Arduino IDE ESP board manager add-on. that can fe found here: https://github.com/esp8266/Arduino
  

  Using the hardware SPI pins is highly recommeneded.
  
  Unzip and change the folder name to "QDTech_8266".
  Copy the folder into your
  <arduinosketchfolder>/libraries/ folder.
  
  You will also need the newest stock "Adafruit_GFX" library.
  https://github.com/adafruit/Adafruit-GFX-Library
  
  The initialisation sequence comes from Henning Karlsen's
  UTFT library.
  
  Includes code from Gilchrist 30/1/2014 and Norm8332 - 10/9/2015

