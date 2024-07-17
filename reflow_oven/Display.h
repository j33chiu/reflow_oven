#ifndef DISPLAY_H
#define DISPLAY_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdarg.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels // change to 64 for larger screen
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32 (large screen may also just use 0x3C)

#define MAX_DISPLAY_BUFF_LEN 256

class Display {

public:
  Display();
  void setup();
  void clear();

  // text printing
  void print(char* format, ...);
  void newline();
  void println(char* format, ...);
  void clear_text();
  void drawText();

private:
  Adafruit_SSD1306 disp;
  char display_buff[MAX_DISPLAY_BUFF_LEN];
  int ptr_pos = 0;

};

#endif