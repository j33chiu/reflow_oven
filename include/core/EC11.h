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

// output rotary events
#define KB_NONE   0x0
#define KB_CW     0x10
#define KB_CCW    0x20

// output switch events
#define SW_NONE     0x0
#define SW_PRESSED  0x10
#define SW_RELEASED 0x20

#define SW_DEBOUNCE_MILLIS 50

// singleton knob class
class EC11 {

public:
  // make assignment and copy inaccessible
  EC11(EC11 const&) = delete;
  EC11(EC11 &&) = delete;
  EC11& operator=(EC11 const&) = delete;
  EC11& operator=(EC11 &&) = delete;

  static EC11& get_instance(const int pin_A = DEFAULT_PIN_A, 
                            const int pin_B = DEFAULT_PIN_B, 
                            const int pin_SW = DEFAULT_PIN_SW, 
                            const int rotary_options = DEFAULT_OPTIONS);
  
  void setup();

  static int get_pin_A();
  static int get_pin_B();
  static int get_pin_SW();
  static int get_encoder_value();

  // set to -1 to allow encoder value to be unlimited
  static void set_rotary_options(const int rotary_options);
  static void set_encoder_value(const int encoder_value);

  static void update_rotate();
  static void update_sw();

  static uint8_t poll_sw_event();
  static uint8_t poll_kb_event();

private:
  EC11(int pin_A = DEFAULT_PIN_A, 
      int pin_B = DEFAULT_PIN_B, 
      int pin_SW = DEFAULT_PIN_SW, 
      int rotary_options = DEFAULT_OPTIONS);

  static const uint8_t rotary_state_table[7][4];

  static int pin_A, pin_B, pin_SW, rotary_options;

  volatile static int encoder_value;

  volatile static uint8_t kb_state, kb_event;

  volatile static bool sw_state;
  volatile static uint8_t sw_event;
  volatile static unsigned long previous_click_millis;
};

#endif