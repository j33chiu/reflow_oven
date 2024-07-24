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

// singleton display class
class Display {

public:
  // make assignment and copy inaccessible
  Display(Display const&) = delete;
  Display(Display &&) = delete; 
  Display& operator=(Display const&) = delete;
  Display& operator=(Display &&) = delete;

  static Display& get_instance();
  static bool is_setup();

  // clearing and reset functions
  static void clear_buff_text();  // clears the text storage buffer
  static void clear_buff_disp();  // clears the current display buffer contents
  static void clear_buffers();    // clears text and current display buffers
  static void clear_full();       // clears all buffers and displays a blank screen

  // text printing
  static bool buff_text_print(const char* format, ...);                                       // writes a string of text to the buffer
  static bool buff_text_endl();                                                               // writes a new line char to the buffer
  static bool buff_text_println(const char* format, ...);                                     // writes a line of text to the buffer, ends with a new line char
  static void draw_text(const int x = 0, const int y = 0, const bool update_screen = true);   // draws current text to display buffer given x and y coords, displaying to the screen by default

  // graph printing
  static void set_graph_lim(const int x_min, const int x_max, const int y_min, const int y_max);  // set graph's numeric limits
  static void set_graph_axes(const int x_axis, const int y_axis);                                 // set axes titles with ints
  static void set_graph_axes(const char* x_axis, const char* y_axis);                             // set axes titles with text
  static void buff_graph_point(const int x, const int y);                                         // draw a point in the graph 
  static void buff_graph_tick(const int val, bool is_x_tick = true);                              // draw a tick on the graph's axes
  static void draw_graph(const bool update_screen = true);                                        // draw full graph to the display buffer, displaying to the screen by default

  // display what is currently in the display buffer
  static void draw();

private:
  Display();

  static Adafruit_SSD1306 disp;
  static bool display_setup;

  // text
  static char display_buff[MAX_DISPLAY_BUFF_LEN + 1];
  static int ptr_pos;

  // graph
  static const int graph_height_px = SCREEN_HEIGHT - (2 * (CHAR_PX_HEIGHT + 2));
  static const int graph_width_px = SCREEN_WIDTH - ((CHAR_PX_WIDTH + 1) * 3 + 1);
  static const int graph_origin_x = (CHAR_PX_WIDTH + 1) * 3 + 1;
  static const int graph_origin_y = SCREEN_HEIGHT - (2 * (CHAR_PX_HEIGHT + 2));

  static int graph_limits[4]; // 0: x_min, 1: x_max, 2: y_min, 3: y_max
  static char graph_x_axis_label[MAX_AXIS_LABEL_LEN + 1]; // max 3 characters
  static char graph_y_axis_label[MAX_AXIS_LABEL_LEN + 1]; // max 3 characters

  static int get_x_px(const int graph_x);
  static int get_y_px(const int graph_y);


};

#endif