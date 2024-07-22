#ifndef DISPLAY_H
#define DISPLAY_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdarg.h>


// defines for the screens, if using 128x32 (small), or 128x63
#define SCREEN_LARGE
//#define SCREEN_SMALL

#define SCREEN_WIDTH 128  // OLED display width, in pixels

#ifdef SCREEN_LARGE
#define SCREEN_HEIGHT 64  // OLED display height, in pixels 
#endif
#ifdef SCREEN_SMALL
#define SCREEN_HEIGHT 32  // in pixels
#endif

#define CHAR_PX_WIDTH   5 // in pixels
#define CHAR_PX_HEIGHT  8 // in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32 (large screen may also just use 0x3C)

#define MAX_DISPLAY_BUFF_LEN 256
#define MAX_AXIS_LABEL_LEN 3

class Display {

public:
  Display();
  void setup();
  void clear();
  void clear_display_buffer();
  void display();

  // text printing
  void print(const char* format, ...);
  void newline();
  void println(const char* format, ...);
  void clear_text();
  void display_text(const int x = 0, const int y = 0, const bool display = true);

  // graph 
  void set_graph_limits(const int x_min, const int x_max, const int y_min, const int y_max);
  void set_axes_labels(const int x_axis, const int y_axis);
  void set_axes_labels(const char* x_axis, const char* y_axis);
  void draw_graph_point(const int x, const int y);
  void draw_graph_tick(const int val, bool is_x_tick = true);
  void display_graph(const bool display = true);

private:
  Adafruit_SSD1306 disp;

  // text
  char display_buff[MAX_DISPLAY_BUFF_LEN + 1];
  int ptr_pos = 0;

  // graph
  const int graph_height_px = SCREEN_HEIGHT - (2 * (CHAR_PX_HEIGHT + 2));
  const int graph_width_px = SCREEN_WIDTH - ((CHAR_PX_WIDTH + 1) * 3 + 1);
  const int graph_origin_x = (CHAR_PX_WIDTH + 1) * 3 + 1;
  const int graph_origin_y = SCREEN_HEIGHT - (2 * (CHAR_PX_HEIGHT + 2));

  int graph_limits[4]; // 0: x_min, 1: x_max, 2: y_min, 3: y_max
  char graph_x_axis_label[MAX_AXIS_LABEL_LEN + 1]; // max 3 characters
  char graph_y_axis_label[MAX_AXIS_LABEL_LEN + 1]; // max 3 characters

  int get_x_px(const int graph_x);
  int get_y_px(const int graph_y);


};

#endif