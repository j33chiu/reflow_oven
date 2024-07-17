#include "TempSensor.h"
#include "Display.h"
#include "SSR.h"
#include "EC11.h"

TempSensor temp_sensor;
Display display;
SSR ssr;
EC11 ec11 = EC11(D0, D1, D2);

void rotate_isr() {
  ec11.update_rotate();
}

void press_isr() {
  ec11.update_sw();
}

void setup() {
  Serial.begin(9600);
  temp_sensor.setup();
  display.setup();

  // ec11 setup isr
  attachInterrupt(digitalPinToInterrupt(ec11.get_pin_A()), rotate_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ec11.get_pin_B()), rotate_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ec11.get_pin_SW()), press_isr, CHANGE);

  // start in idle
  enter_idle_state();
}


// Loop state machine
#define IDLE    0       // choose heating profile (knob + button), or wait for BT commands (wireless)
#define REFLOWING     1       // entire reflow profile, skips individual zones from typical reflow profiles, follows precisely set profile
#define MANUAL        10      // following manually set heating temperature target, eg via knob (meant as a debug mode)
#define DEBUG         20      // debug mode

uint8_t oven_state = IDLE;

#define PROFILE_CHOICES 3
#define MAX_PROFILE_CHOICES 5

void idle_loop() {
  // poll the EC11 object for the currently selected object
  int choice = ec11.get_encoder_value();
  // loop to display reflow profile choices
  display.clear_text();
  for (int i = 0; i < min(PROFILE_CHOICES, MAX_PROFILE_CHOICES); i++) {
    display.println("%s Profile %d", choice == i ? "[*]" : "[ ]", i+1);
  }
  display.println("%s Manual Mode", choice == PROFILE_CHOICES ? "[*]" : "[ ]");
  display.println("%s Debug Mode", choice == (PROFILE_CHOICES + 1) ? "[*]" : "[ ]");
  display.drawText();

  // select and change states depending on selection
  switch(ec11.poll_sw_event()) {
    case SW_PRESSED:
      switch(choice) {
        case PROFILE_CHOICES:
          // manual mode
          enter_manual_state();
          break;
        case PROFILE_CHOICES + 1:
          // debug mode
          enter_debug_state();
          break;
        default:
          // reflow profile
          enter_reflow_state(choice);
          break;
      }
      break;
    case SW_RELEASED:
      break;
    case SW_NONE:
    default:
      break;
  }
}

void reflow_loop() {
  // update the ovencontroller to update pid and ssr duty cycle

  // if clicked, emergency stop and return to idle state
  switch(ec11.poll_sw_event()) {
    case SW_PRESSED:
      ssr.emer_stop();
      enter_idle_state();
      break;
    case SW_RELEASED:
      break;
    case SW_NONE:
    default:
      break;
  }
}

void manual_loop() {
  float target_temp = ec11.get_encoder_value();
  display.clear_text();
  display.println("Target Temp: %06.2f C", target_temp);
  display.println("\nClick knob to exit");
  display.drawText();

  // if clicked, return to idle state
  switch(ec11.poll_sw_event()) {
    case SW_PRESSED:
      enter_idle_state();
      break;
    case SW_RELEASED:
      break;
    case SW_NONE:
    default:
      break;
  }
}

void debug_loop() {
  enter_idle_state();
}

void loop() {
  switch(oven_state) {
    case IDLE_STATE:
      idle_loop();
      break;
    case REFLOWING:
      reflow_loop();
      break;
    case MANUAL:
      manual_loop();
      break;
    case DEBUG:
    default:
      debug_loop();
      break;
  }


  // // update temperature
  // temp_sensor.update();
  // float sensor_temp = -1;

  // if (temp_sensor.getNumDevices() == 0) {
  //   clear_display_buff();
  //   snprintf(display_buff, MAX_DISPLAY_BUFF_LEN, "%s", "No Temperature Sensors Found...");
  //   display.drawText(display_buff);
  //   delay(5000);
  // } else {
  //   // get address of sensor 0
  //   sensor_temp = temp_sensor.getTempCByIndex(0);
  //   // clear_display_buff();
  //   // snprintf(display_buff, MAX_DISPLAY_BUFF_LEN, "Temperature: %06.2f", sensor_temp);
  //   // display.drawText(display_buff);
  // }

  // // if invalid temperature, shut off
  // if (sensor_temp < 10.0f || sensor_temp > 400.0f) {
  //   ssr.emer_stop();
  //   clear_display_buff();
  //   snprintf(display_buff, MAX_DISPLAY_BUFF_LEN, "%s", "Invalid temperature encountered, please reset system...");
  //   display.drawText(display_buff);
  //   delay(5000);
  // } else {
  //   ssr.set_on();
  // }


}

// functions to change states, along with any necessary setup before entering the states
void enter_idle_state() {
  ec11.set_rotary_options(PROFILE_CHOICES + 2); // +2 for manual and debug choices
  ec11.set_encoder_value(0);
  oven_state = IDLE;
}

void enter_reflow_state(int profile_choice) {
  Serial.print("chose profile ");
  Serial.println(profile_choice);
  ssr.reset();
  oven_state = REFLOWING;
}

void enter_manual_state() {
  // set encoder to be limitless on value
  ec11.set_rotary_options(-1);
  ec11.set_encoder_value(20); // TODO: set to current temp?
  oven_state = MANUAL;
}

void enter_debug_state() {
  Serial.print("Entering Debug state");
  oven_state = DEBUG;
}