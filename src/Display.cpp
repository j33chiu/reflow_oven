#include "Display.h"

Display::Display() {
  this->disp = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

void Display::setup() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!disp.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  disp.clearDisplay();
  disp.display();
}

void Display::clear() {
  disp.clearDisplay();
  disp.display();
}

void Display::clear_display_buffer() {
  disp.clearDisplay();
}

void Display::display() {
  disp.display();
}

void Display::print(const char* format, ...) {
  if (ptr_pos >= 0 && ptr_pos < MAX_DISPLAY_BUFF_LEN) {
    va_list va;
    va_start(va, format);
    ptr_pos += vsnprintf(display_buff + ptr_pos, MAX_DISPLAY_BUFF_LEN - ptr_pos, format, va);
    va_end(va);
  }
}

void Display::newline() {
  if (ptr_pos >= 0 && ptr_pos < MAX_DISPLAY_BUFF_LEN) {
    ptr_pos += snprintf(display_buff + ptr_pos, MAX_DISPLAY_BUFF_LEN - ptr_pos, "\n");
  }
}

void Display::println(const char* format, ...) {
  if (ptr_pos >= 0 && ptr_pos < MAX_DISPLAY_BUFF_LEN) {
    va_list va;
    va_start(va, format);
    ptr_pos += vsnprintf(display_buff + ptr_pos, MAX_DISPLAY_BUFF_LEN - ptr_pos, format, va);
    va_end(va);
  }
  newline();
}

void Display::clear_text() {
  ptr_pos = 0;
  memset(display_buff, 0, sizeof(display_buff));
}

void Display::display_text(const int x, const int y, const bool display) {
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);        // Draw white text
  disp.setCursor(x, y);
  for (int i = 0; i < MAX_DISPLAY_BUFF_LEN - 1; i++) {
    disp.write(display_buff[i]);
  }
  if (display) disp.display();
}

void Display::set_graph_limits(const int x_min, const int x_max, const int y_min, const int y_max) {
  this->graph_limits[0] = x_min;
  this->graph_limits[1] = x_max;
  this->graph_limits[2] = y_min;
  this->graph_limits[3] = y_max;
}

void Display::set_axes_labels(const int x_axis, const int y_axis) {
  sniprintf(graph_x_axis_label, MAX_AXIS_LABEL_LEN + 1, "%.3d", x_axis);
  sniprintf(graph_y_axis_label, MAX_AXIS_LABEL_LEN + 1, "%.3d", y_axis);
}

void Display::set_axes_labels(const char* x_axis, const char* y_axis) {
  snprintf(graph_x_axis_label, MAX_AXIS_LABEL_LEN + 1, x_axis);
  snprintf(graph_y_axis_label, MAX_AXIS_LABEL_LEN + 1, y_axis);
}

void Display::draw_graph_point(const int x, const int y) {
  int x_px = get_x_px(x);
  int y_px = get_y_px(y);
  disp.drawPixel(x_px, y_px, SSD1306_WHITE);
  disp.drawPixel(x_px + 1, y_px - 1, SSD1306_WHITE);
  disp.drawPixel(x_px + 1, y_px + 1, SSD1306_WHITE);
  disp.drawPixel(x_px - 1, y_px - 1, SSD1306_WHITE);
  disp.drawPixel(x_px - 1, y_px + 1, SSD1306_WHITE);
}

void Display::draw_graph_tick(const int val, const bool is_x_tick) {
  if (is_x_tick) {
    disp.drawFastVLine(get_x_px(val), graph_origin_y - 1, 3, SSD1306_WHITE);
  } else {
    disp.drawFastHLine(graph_origin_x - 1, get_y_px(val), 3, SSD1306_WHITE);
  }
}

void Display::display_graph(const bool display) {
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


  if (display) disp.display();
}

int Display::get_x_px(const int graph_x) {
  return min(SCREEN_WIDTH - 1, graph_origin_x + (int)(graph_x * ((SCREEN_WIDTH - graph_origin_x) / (float)(graph_limits[1] - graph_limits[0]))));
}

int Display::get_y_px(const int graph_y) {
  return max(0, graph_origin_y - (int)(graph_y * (graph_origin_y / (float)(graph_limits[3] - graph_limits[2]))));
}