#ifndef EC11_H
#define EC11_H

#include <Arduino.h>

#define DEFAULT_PIN_A     D0
#define DEFAULT_PIN_B     D1
#define DEFAULT_PIN_SW    D2
#define DEFAULT_OPTIONS   10

// turning states
#define START     0x0
#define CW_1      0x1
#define CW_MID    0x2
#define CW_2      0x3
#define CCW_1     0x4
#define CCW_MID   0x5
#define CCW_2     0x6

// output rotary states
#define CW_TURN   0x10
#define CCW_TURN  0x20

// output switch states
#define SW_NONE     0x0
#define SW_PRESSED  0x10
#define SW_RELEASED 0x20

const uint8_t rotary_state_table[7][4] = {
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

#define SW_DEBOUNCE_MILLIS 50

class EC11 {

public:
  EC11(int pin_A = DEFAULT_PIN_A, 
       int pin_B = DEFAULT_PIN_B, 
       int pin_SW = DEFAULT_PIN_SW, 
       int rotary_options = DEFAULT_OPTIONS);

  int get_pin_A();
  int get_pin_B();
  int get_pin_SW();
  int get_encoder_value();

  // set to -1 to allow encoder value to be unlimited
  void set_rotary_options(int rotary_options);
  void set_encoder_value(int encoder_value);

  void update_rotate();
  void update_sw();

  uint8_t poll_sw_event();

private:
  int pin_A = DEFAULT_PIN_A;
  int pin_B = DEFAULT_PIN_B;
  int pin_SW = DEFAULT_PIN_SW;
  
  int rotary_options = DEFAULT_OPTIONS;
  volatile int encoder_value = 0;

  volatile uint8_t state = 0x0;

  volatile bool sw_state = false;
  volatile uint8_t sw_event = SW_NONE;
  volatile unsigned long previous_click_millis = 0;
};

#endif