#include <Arduino.h>

#include "TempSensor.h"
#include "Display.h"
#include "SSR.h"
#include "EC11.h"
#include "ProfileEditor.h"
#include "OvenController.h"

#include <Preferences.h>
#include <nvs_flash.h>
Preferences pref;

// control ui and screens
OvenController oven_controller();

// Loop state machine
#define IDLE            0 
#define PROFILES_LIST   1       // list all available profiles, with +new profile option up to MAX_PROFILE_CHOICES
#define PROFILE_OPTIONS 2       // edit, delete or run options for the currently selected profile
#define REFLOWING       3       // entire reflow profile, skips individual zones from typical reflow profiles, follows precisely set profile
#define MANUAL          10      // following manually set heating temperature target, eg via knob (meant as a debug mode)
#define DEBUG           20      // debug mode

uint8_t oven_state = IDLE;

#define IDLE_OPTIONS        3   // profile list, manual mode, debug mode

// functions to change states, along with any necessary setup before entering the states
void enter_idle_state() {
  pref.end();     // if exiting from profiles list state, we want to close preferences.
  EC11::set_rotary_options(IDLE_OPTIONS);
  EC11::set_encoder_value(0);

  // display screen, start at choice 0 so can be hardcoded
  Display::clear_buffers();
  Display::buff_text_println("[*] Profiles");
  Display::buff_text_println("[ ] Manual Mode");
  Display::buff_text_println("[ ] Debug Mode");
  Display::draw_text();

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
  EC11::set_rotary_options(num_profiles + 1 + (num_profiles == 5 ? 0 : 1));
  EC11::set_encoder_value(0);
  oven_state = PROFILES_LIST;
}

void enter_reflow_state(int profile_choice) {
  SSR::get_instance().reset();

  // start reflow process with OvenController


  oven_state = REFLOWING;
}

void enter_manual_state() {
  // set encoder to be limitless on value
  EC11::set_rotary_options(-1);
  EC11::set_encoder_value(20); // TODO: set to current temp?
  oven_state = MANUAL;
}

void enter_debug_state() {
  EC11::set_rotary_options(-1);
  oven_state = DEBUG;
}

void setup() {
  TempSensor::get_instance().setup();   // setup temperature sensor

  // display setup
  if (!Display::get_instance().is_setup()) {
    // display did not setup properly
    while (true) {
      delay(500);
    }
  }

  // ec11 knob setup
  EC11::get_instance(D0, D1, D2).setup();

  // start in idle
  enter_idle_state();

  // flash storage
  nvs_flash_init();
}

void idle_loop() {
  // poll the EC11 object for the currently selected object
  int choice = EC11::get_encoder_value();

  // poll knob, if turned, update the screen
  if (EC11::poll_kb_event() != KB_NONE) {
    Display::clear_buffers();
    Display::buff_text_println("%s Profiles", choice == 0 ? "[*]" : "[ ]");
    Display::buff_text_println("%s Manual Mode", choice == 1 ? "[*]" : "[ ]");
    Display::buff_text_println("%s Debug Mode", choice == 2 ? "[*]" : "[ ]");
    Display::draw_text();
  }

  // select and change states depending on selection
  if (EC11::poll_sw_event() == SW_PRESSED) {
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
  int choice = EC11::get_encoder_value();

  Display::clear_buffers();
  // print profiles
  int num_profiles = 0;
  for (int i = 0; i < MAX_PROFILE_CHOICES; i++) {
    if (!pref.isKey(profile_keys[i * 2])) break;
    // key is valid, get the name of the profile
    Display::buff_text_println("%s %s", choice == i ? "[*]" : "[ ]", pref.getString(profile_keys[i*2]).c_str());
    ++num_profiles;
  }
  if (num_profiles == 0) {
    Display::buff_text_println("[ ] <no profiles>");
  }
  // print +profile option if necessary
  if (num_profiles < 5) {
    Display::buff_text_println("%s [+]", choice == num_profiles ? "[*]" : "[ ]");
  }
  // print back option
  Display::buff_text_println("%s Back", choice == min(num_profiles + 1, MAX_PROFILE_CHOICES) ? "[*]" : "[ ]");
  Display::draw_text();

  // select and change states depending on selection
  if (EC11::poll_sw_event() == SW_PRESSED) {
    if (choice < num_profiles) {
      // chose an existing profile
      ProfileEditor editor(&pref);
      switch(editor.edit_profile(choice)) {
        case PROFILE_START: enter_reflow_state(choice); break;
        case PROFILE_CHANGED: 
          pref.end(); // pref was set to read/write, end so we can restart it with just read
          enter_profiles_state();
          break;
        case PROFILE_NOTHING: 
        default: enter_profiles_state(); break;
      }
    } else if (choice == num_profiles) {
      // chose to add a new profile
      ProfileEditor editor(&pref);
      if (editor.new_profile()) {
        // pref was set to read/write, end so we can restart it with just read
        pref.end();
      }
      // done with profile editor
      enter_profiles_state();
    } else {
      // back
      enter_idle_state();
    }
  }
}

void reflow_loop() {
  // TODO: update the ovencontroller to update pid and ssr duty cycle

  // if clicked, emergency stop and return to idle state
  switch(EC11::poll_sw_event()) {
    case SW_PRESSED:
      SSR::get_instance().emer_stop();
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
  float target_temp = EC11::get_encoder_value();

  Display::clear_buffers();
  Display::buff_text_println("Target Temp: %06.2f C", target_temp);
  Display::buff_text_println("\nClick knob to exit");
  Display::draw_text();

  // if clicked, return to idle state
  if (EC11::poll_sw_event() == SW_PRESSED) {
    enter_idle_state();
  }
}

void debug_loop() {
  // debug temperature sensor
  TempSensor::get_instance().update();
  float sensor_temp = -1;

  Display::clear_buffers();
  if (TempSensor::get_instance().getNumDevices() == 0) {
    Display::buff_text_println("No Temperature Sensors Found...");
  } else {
    // get address of sensor 0
    sensor_temp = TempSensor::get_instance().getTempCByIndex(0);
    Display::buff_text_println("Temperature: %06.2f", sensor_temp);
  }
  Display::buff_text_println("\nClick knob to exit");
  Display::draw_text();

  // if clicked, return to idle state
  if (EC11::poll_sw_event() == SW_PRESSED) {
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
  delay(50);
}