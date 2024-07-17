#include "EC11.h"

EC11::EC11(int pin_A, int pin_B, int pin_SW, int rotary_options) {
  this->rotary_options = rotary_options;
  this->pin_A = pin_A;
  this->pin_B = pin_B;
  this->pin_SW = pin_SW;
  pinMode(pin_A, INPUT_PULLUP);
  pinMode(pin_B, INPUT_PULLUP);
  pinMode(pin_SW, INPUT_PULLUP);
}

int EC11::get_pin_A() {
  return this->pin_A;
}

int EC11::get_pin_B() {
  return this->pin_B;
}

int EC11::get_pin_SW() {
  return this->pin_SW;
}

int EC11::get_encoder_value() {
  return this->encoder_value;
}

void EC11::set_rotary_options(int rotary_options) {
  this->rotary_options = rotary_options;
}

void EC11::set_encoder_value(int encoder_value) {
  this->encoder_value = encoder_value;
}

// state machine from https://github.com/buxtronix/arduino/tree/master/libraries/Rotary 
void EC11::update_rotate() {
  uint8_t current_state = (digitalRead(this->pin_B) << 1) | digitalRead(this->pin_A);
  this->state = rotary_state_table[state & 0xf][current_state];
  
  if (this->state == CW_TURN) {
    if (rotary_options < 0) {
      ++encoder_value;
    } else {
      encoder_value = (encoder_value + 1) % rotary_options;
    }
  } else if (this->state == CCW_TURN) {
    if (rotary_options < 0) {
      --encoder_value;
    } else {
      encoder_value = (encoder_value - 1) < 0 ? (rotary_options - 1) : ((encoder_value - 1) % rotary_options); 
    }
  }
}

void EC11::update_sw() {
  // software debouncing
  if (millis() - this->previous_click_millis < SW_DEBOUNCE_MILLIS) {
    return;
  }
  // debounce ok, read button state
  bool current_state = digitalRead(this->pin_SW);
  current_state = !current_state;
  if (current_state == this->sw_state) { // same state, ignore
    return;
  }
  // update debounce and current state, add event.
  this->previous_click_millis = millis();
  this->sw_state = current_state;
  this->sw_event = this->sw_state ? SW_PRESSED : SW_RELEASED;
}

uint8_t EC11::poll_sw_event() {
  uint8_t event = this->sw_event;
  this->sw_event = SW_NONE;

  return event;
}