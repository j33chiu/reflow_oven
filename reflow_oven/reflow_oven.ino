#include "TempSensor.h"
#include "Display.h"
#include "SSR.h"
#include "EC11.h"

#include <Preferences.h>
#define PREF_NAMESPACE "pref_rf_oven"
Preferences pref;

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
#define IDLE            0 
#define PROFILES_LIST   1       // list all available profiles, with +new profile option up to MAX_PROFILE_CHOICES
#define PROFILE_OPTIONS 2       // edit, delete or run options for the currently selected profile
#define REFLOWING       3       // entire reflow profile, skips individual zones from typical reflow profiles, follows precisely set profile
#define MANUAL          10      // following manually set heating temperature target, eg via knob (meant as a debug mode)
#define DEBUG           20      // debug mode

uint8_t oven_state = IDLE;

#define IDLE_OPTIONS        3   // profile list, manual mode, debug mode
#define MAX_PROFILE_CHOICES 5   // max profiles that can be saved

// access profile info from preferences
const char profile_keys[MAX_PROFILE_CHOICES * 2][3] = { // 'n' is the profile name, 'p' is the actual profile data
  "n1", "p1",
  "n2", "p2",
  "n3", "p3",
  "n4", "p4",
  "n5", "p5",
};

void idle_loop() {
  // poll the EC11 object for the currently selected object
  int choice = ec11.get_encoder_value();
  // loop to display reflow profile choices
  display.clear_text();
  display.println("%s Profiles", choice == 0 ? "[*]" : "[ ]");
  display.println("%s Manual Mode", choice == 1 ? "[*]" : "[ ]");
  display.println("%s Debug Mode", choice == 2 ? "[*]" : "[ ]");
  display.drawText();

  // select and change states depending on selection
  if (ec11.poll_sw_event() == SW_PRESSED) {
    switch(choice) {
      case 0: enter_profiles_state(); break;  // profiles
      case 1: enter_manual_state(); break;    // manual mode
      case 2: enter_debug_state(); break;     // debug mode
      default: break;
    }
  }
}

void profiles_list_loop() {
  // poll the EC11 object for the currently selected object
  int choice = ec11.get_encoder_value();

  display.clear_text();
  // print profiles
  int num_profiles = 0;
  for (int i = 0; i < MAX_PROFILE_CHOICES; i++) {
    if (!pref.isKey(profile_keys[i * 2])) break;
    // key is valid, get the name of the profile
    display.println("%s %s", choice == i ? "[*]" : "[ ]", pref.getString(profile_keys[i*2]).c_str());
    ++num_profiles;
  }
  if (num_profiles == 0) {
    display.println("[ ] <no profiles>");
  }
  // print +profile option if necessary
  if (num_profiles < 5) {
    display.println("%s [+]", choice == num_profiles ? "[*]" : "[ ]");
  }
  // print back option
  display.println("%s Back", choice == min(num_profiles + 1, MAX_PROFILE_CHOICES) ? "[*]" : "[ ]");
  display.drawText();

  // select and change states depending on selection
  if (ec11.poll_sw_event() == SW_PRESSED) {
    if (choice < num_profiles) {
      // chose an existing profile
    } else if (choice == num_profiles) {
      // chose to add a new profile
    } else {
      // back
      enter_idle_state();
    }
  }
}

void reflow_loop() {
  // TODO: update the ovencontroller to update pid and ssr duty cycle

  // if clicked, emergency stop and return to idle state
  switch(ec11.poll_sw_event()) {
    case SW_PRESSED:
      ssr.emer_stop();
      // TODO: stop process in OvenController
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
  if (ec11.poll_sw_event() == SW_PRESSED) {
    enter_idle_state();
  }
}

void debug_loop() {
  // debug temperature sensor
  temp_sensor.update();
  float sensor_temp = -1;

  display.clear_text();
  if (temp_sensor.getNumDevices() == 0) {
    display.println("No Temperature Sensors Found...");
  } else {
    // get address of sensor 0
    sensor_temp = temp_sensor.getTempCByIndex(0);
    display.println("Temperature: %06.2f", sensor_temp);
  }
  display.println("\nClick knob to exit");
  display.drawText();

  // if clicked, return to idle state
  if (ec11.poll_sw_event() == SW_PRESSED) {
    enter_idle_state();
  }
}

void loop() {
  switch(oven_state) {
    case IDLE:
      idle_loop();
      break;
    case PROFILES_LIST:
      profiles_list_loop();
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
}

// functions to change states, along with any necessary setup before entering the states
void enter_idle_state() {
  pref.end();     // if exiting from profiles list state, we want to close preferences.
  ec11.set_rotary_options(IDLE_OPTIONS);
  ec11.set_encoder_value(0);
  oven_state = IDLE;
}

void enter_profiles_state() {
  // when we enter this state, we start with reads. any subsequent write operation must pref.end(), 
  // then restart with write permissions, finish writing, then restart with read permissions. 
  // the final pref.end() will be called when we go back to the idle mode
  pref.begin(PREF_NAMESPACE, true); 
  // get number of profiles
  int num_profiles = 0;
  for (int i = 0; i < MAX_PROFILE_CHOICES; i++) {
    if (!pref.isKey(profile_keys[i * 2])) break;
    ++num_profiles;
  }
  //                      // profiles    //back option    //new profile option
  ec11.set_rotary_options(num_profiles + 1 + (num_profiles == 5 ? 0 : 1));
  ec11.set_encoder_value(0);
  oven_state = PROFILES_LIST;
}

void enter_reflow_state(int profile_choice) {
  Serial.print("chose profile ");
  Serial.println(profile_choice);
  ssr.reset();

  // start reflow process with OvenController

  oven_state = REFLOWING;
}

void enter_manual_state() {
  // set encoder to be limitless on value
  ec11.set_rotary_options(-1);
  ec11.set_encoder_value(20); // TODO: set to current temp?
  oven_state = MANUAL;
}

void enter_debug_state() {
  ec11.set_rotary_options(-1);
  oven_state = DEBUG;
}