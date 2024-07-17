#include "Display.h"

Display::Display() {
  this->disp = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

void Display::setup() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!disp.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  disp.clearDisplay();
  disp.display();
}

void Display::clear() {
  disp.clearDisplay();
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

void Display::drawText() {
  disp.clearDisplay();
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);        // Draw white text
  disp.setCursor(0,0);             // Start at top-left corner
  for (int i = 0; i < MAX_DISPLAY_BUFF_LEN - 1; i++) {
    disp.write(display_buff[i]);
  }
  disp.display();
}
