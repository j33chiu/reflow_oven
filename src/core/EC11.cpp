#include "core/EC11.h"

// static initializations
const uint8_t EC11::rotary_state_table[7][4] = {
  // START
  // 00:  01:   10:    11:    
  {START, CW_1, CCW_1, START},
  // CW_1
  // 00:   01:   10:    11:
  {CW_MID, CW_1, START, START},
  // CW_MID
  // 00:   01:   10:   11:
  {CW_MID, CW_1, CW_2, START},
  // CW_2
  // 00:    01:    10:  11:
  {CW_MID, START, CW_2, CW_TURN},
  // CCW_1
  // 00:    01:    10:    11:
  {CCW_MID, START, CCW_1, START},
  // CCW_MID
  // 00:    01:    10:    11:
  {CCW_MID, CCW_2, CCW_1, START},
  // CCW_2
  // 00:    01:    10:    11:
  {CCW_MID, CCW_2, START, CCW_TURN}
};

int EC11::pin_A = DEFAULT_PIN_A;
int EC11::pin_B = DEFAULT_PIN_B;
int EC11::pin_SW = DEFAULT_PIN_SW;
int EC11::rotary_options = DEFAULT_OPTIONS;

volatile int EC11::encoder_value = 0;

volatile uint8_t EC11::kb_state = 0x0;
volatile uint8_t EC11::kb_event = KB_NONE;

volatile bool EC11::sw_state = false;
volatile uint8_t EC11::sw_event = SW_NONE;
volatile unsigned long EC11::previous_click_millis = 0;

EC11& EC11::get_instance(const int pin_A, 
                         const int pin_B,
                         const int pin_SW,
                         const int rotary_options) 
{
  static EC11 instance(pin_A, pin_B, pin_SW, rotary_options);
  return instance;  
}

EC11::EC11(int pin_A, int pin_B, int pin_SW, int rotary_options) {
  EC11::rotary_options = rotary_options;
  EC11::pin_A = pin_A;
  EC11::pin_B = pin_B;
  EC11::pin_SW = pin_SW;
  pinMode(pin_A, INPUT_PULLUP);
  pinMode(pin_B, INPUT_PULLUP);
  pinMode(pin_SW, INPUT_PULLUP);
}

void EC11::setup() {
  attachInterrupt(digitalPinToInterrupt(get_pin_A()), update_rotate, CHANGE);
  attachInterrupt(digitalPinToInterrupt(get_pin_B()), update_rotate, CHANGE);
  attachInterrupt(digitalPinToInterrupt(get_pin_SW()), update_sw, CHANGE);
}

int EC11::get_pin_A() {
  return pin_A;
}

int EC11::get_pin_B() {
  return pin_B;
}

int EC11::get_pin_SW() {
  return pin_SW;
}

int EC11::get_encoder_value() {
  return encoder_value;
}

void EC11::set_rotary_options(const int rotary_options) {
  EC11::rotary_options = rotary_options;
}

void EC11::set_encoder_value(int encoder_value) {
  EC11::encoder_value = encoder_value;
}

// state machine from https://github.com/buxtronix/arduino/tree/master/libraries/Rotary 
void EC11::update_rotate() {
  uint8_t current_state = (digitalRead(pin_B) << 1) | digitalRead(pin_A);
  kb_state = rotary_state_table[kb_state & 0xf][current_state];
  
  if (kb_state == CW_TURN) {
    if (rotary_options < 0) {
      ++encoder_value; // no options, knob acting as unbounded value selection
    } else {
      encoder_value = (encoder_value + 1) % rotary_options;
      kb_event = KB_CW;
    }
  } else if (kb_state == CCW_TURN) {
    if (rotary_options < 0) {
      --encoder_value; // no options, knob acting as unbounded value selection
    } else {
      encoder_value = (encoder_value - 1) < 0 ? (rotary_options - 1) : ((encoder_value - 1) % rotary_options); 
      kb_event = KB_CCW;
    }
  }
}

void EC11::update_sw() {
  // software debouncing
  if (millis() - previous_click_millis < SW_DEBOUNCE_MILLIS) {
    return;
  }
  // debounce ok, read button state
  bool current_state = digitalRead(pin_SW);
  current_state = !current_state;
  if (current_state == sw_state) { // same state, ignore
    return;
  }
  // update debounce and current state, add event.
  previous_click_millis = millis();
  sw_state = current_state;
  sw_event = sw_state ? SW_PRESSED : SW_RELEASED;
}

uint8_t EC11::poll_sw_event() {
  uint8_t event = sw_event;
  sw_event = SW_NONE;

  return event;
}

uint8_t EC11::poll_kb_event() {
  uint8_t event = kb_event;
  kb_event = KB_NONE;

  return event;
}