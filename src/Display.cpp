#include "Display.h"

// static initializations
bool Display::display_setup = false;
Adafruit_SSD1306 Display::disp;
char Display::display_buff[MAX_DISPLAY_BUFF_LEN + 1];
int Display::ptr_pos;
int Display::graph_limits[4];
char Display::graph_x_axis_label[MAX_AXIS_LABEL_LEN + 1];
char Display::graph_y_axis_label[MAX_AXIS_LABEL_LEN + 1];

Display& Display::get_instance() {
  static Display instance;
  return instance;
}

Display::Display() {
  disp = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  // auto setup
  display_setup = disp.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  if (display_setup) {
    disp.clearDisplay();
    disp.display();
  }
  ptr_pos = 0; // set text buffer ptr to 0
}

bool Display::is_setup() {
  return display_setup;
}

void Display::clear_buff_text() {
  ptr_pos = 0;
  memset(display_buff, 0, sizeof(display_buff));
}

void Display::clear_buff_disp() {
  disp.clearDisplay();
}

void Display::clear_buffers() {
  clear_buff_text();
  clear_buff_disp();
}

void Display::clear_full() {
  clear_buffers();
  disp.display();
}

bool Display::buff_text_print(const char* format, ...) {
  if (ptr_pos >= 0 && ptr_pos < MAX_DISPLAY_BUFF_LEN) {
    va_list va;
    va_start(va, format);
    ptr_pos += vsnprintf(display_buff + ptr_pos, MAX_DISPLAY_BUFF_LEN - ptr_pos, format, va);
    va_end(va);
    return true;
  }
  return false;
}

bool Display::buff_text_endl() {
  if (ptr_pos >= 0 && ptr_pos < MAX_DISPLAY_BUFF_LEN) {
    ptr_pos += snprintf(display_buff + ptr_pos, MAX_DISPLAY_BUFF_LEN - ptr_pos, "\n");
    return true;
  }
  return false;
}

bool Display::buff_text_println(const char* format, ...) {
  if (ptr_pos >= 0 && ptr_pos < MAX_DISPLAY_BUFF_LEN) {
    va_list va;
    va_start(va, format);
    ptr_pos += vsnprintf(display_buff + ptr_pos, MAX_DISPLAY_BUFF_LEN - ptr_pos, format, va);
    va_end(va);
    return buff_text_endl();
  }
  return false;
}

void Display::draw_text(const int x, const int y, const bool update_screen) {
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);        // Draw white text
  disp.setCursor(x, y);
  for (int i = 0; i < MAX_DISPLAY_BUFF_LEN - 1; i++) {
    disp.write(display_buff[i]);
  }
  if (update_screen) draw();
}

void Display::set_graph_lim(const int x_min, const int x_max, const int y_min, const int y_max) {
  graph_limits[0] = x_min;
  graph_limits[1] = x_max;
  graph_limits[2] = y_min;
  graph_limits[3] = y_max;
}

void Display::set_graph_axes(const int x_axis, const int y_axis) {
  sniprintf(graph_x_axis_label, MAX_AXIS_LABEL_LEN + 1, "%.3d", x_axis);
  sniprintf(graph_y_axis_label, MAX_AXIS_LABEL_LEN + 1, "%.3d", y_axis);
}

void Display::set_graph_axes(const char* x_axis, const char* y_axis) {
  snprintf(graph_x_axis_label, MAX_AXIS_LABEL_LEN + 1, x_axis);
  snprintf(graph_y_axis_label, MAX_AXIS_LABEL_LEN + 1, y_axis);
}

void Display::buff_graph_point(const int x, const int y) {
  int x_px = get_x_px(x);
  int y_px = get_y_px(y);
  disp.drawPixel(x_px, y_px, SSD1306_WHITE);
  disp.drawPixel(x_px + 1, y_px - 1, SSD1306_WHITE);
  disp.drawPixel(x_px + 1, y_px + 1, SSD1306_WHITE);
  disp.drawPixel(x_px - 1, y_px - 1, SSD1306_WHITE);
  disp.drawPixel(x_px - 1, y_px + 1, SSD1306_WHITE);
}

void Display::buff_graph_tick(const int val, const bool is_x_tick) {
  if (is_x_tick) {
    disp.drawFastVLine(get_x_px(val), graph_origin_y - 1, 3, SSD1306_WHITE);
  } else {
    disp.drawFastHLine(graph_origin_x - 1, get_y_px(val), 3, SSD1306_WHITE);
  }
}

void Display::draw_graph(const bool update_screen) {
  // axes lines
  // x axis, allow space for text 
  disp.drawFastHLine(graph_origin_x, graph_origin_y, graph_width_px, SSD1306_WHITE);
  // y axis, allow space for text
  disp.drawFastVLine(graph_origin_x, 0, graph_height_px, SSD1306_WHITE);

  // draw axis labels
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);        // Draw white text
  disp.setCursor(0, (SCREEN_HEIGHT - CHAR_PX_HEIGHT) / 2);
  for (int i = 0; i < MAX_AXIS_LABEL_LEN; i++) {
    disp.write(graph_y_axis_label[i]);
  }
  disp.setCursor((SCREEN_WIDTH - (CHAR_PX_WIDTH + 1)) / 2, graph_origin_y + 2); // leave 1 pixel between axes and text
  for (int i = 0; i < MAX_AXIS_LABEL_LEN; i++) {
    disp.write(graph_x_axis_label[i]);
  }
  if (update_screen) disp.display();
}

void Display::draw() {
  disp.display();
}

int Display::get_x_px(const int graph_x) {
  return min(SCREEN_WIDTH - 1, graph_origin_x + (int)(graph_x * ((SCREEN_WIDTH - graph_origin_x) / (float)(graph_limits[1] - graph_limits[0]))));
}

int Display::get_y_px(const int graph_y) {
  return max(0, graph_origin_y - (int)(graph_y * (graph_origin_y / (float)(graph_limits[3] - graph_limits[2]))));
}