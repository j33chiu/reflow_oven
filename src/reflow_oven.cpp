#include <Arduino.h>

#include "core/TempSensor.h"
#include "core/Display.h"
#include "core/SSR.h"
#include "core/EC11.h"
#include "core/Storage.h"

#include "screens/ScreenStates.h"
#include "screens/IdleScreen.h"
#include "screens/ManualScreen.h"
#include "screens/DebugScreen.h"
#include "screens/ProfileListScreen.h"
#include "screens/ProfileScreen.h"
#include "screens/ProfileEditorScreen.h"
#include "OvenController.h"

// control ui and screens
OvenController oven_controller;

uint8_t oven_state = STATE_IDLE;

// functions to change states, along with any necessary setup before entering the states
void enter_idle_state() {
  IdleScreen::get_instance().start();
  oven_state = STATE_IDLE;
}

void enter_profiles_state() {
  ProfileListScreen::get_instance().start();
  oven_state = STATE_PROFILES_LIST;
}

void enter_profile_options_state(uint8_t profile_idx) {
  ProfileScreen::get_instance().start(profile_idx);
  oven_state = STATE_PROFILE_OPTIONS;
}

void enter_reflow_state(int profile_choice) {
  SSR::get_instance().reset();

  // start reflow process with OvenController
  oven_state = STATE_REFLOWING;
}

void enter_manual_state() {
  ManualScreen::get_instance().start();
  oven_state = STATE_MANUAL;
}

void enter_debug_state() {
  DebugScreen::get_instance().start();
  oven_state = STATE_DEBUG;
}

void setup() {
  // setup temperature sensor
  TempSensor::get_instance().setup();

  // display setup
  if (!Display::get_instance().is_setup()) {
    // display did not setup properly
    while (true) {
      delay(500);
    }
  }

  // ec11 knob setup
  EC11::get_instance(D0, D1, D2).setup();

  // flash storage initialization
  if (!Storage::get_instance().setup()) {
    while (true) {
      delay(500);
    }
  }

  // start in idle
  enter_idle_state();
}

void idle_loop() {
  switch (IdleScreen::get_instance().loop()) {
    case IDLE_OPTION_PROFILES: enter_profiles_state(); break;  // profiles
    case IDLE_OPTION_MANUAL: enter_manual_state(); break;      // manual mode
    case IDLE_OPTION_DEBUG: enter_debug_state(); break;        // debug mode
    case OPTION_NONE:
    default: break;
  }
}

void profiles_list_loop() {
  int result = ProfileListScreen::get_instance().loop();
  int result_data = result & 0xf;
  result = result & 0xfff0;

  switch (result) {
    case PROFILES_CHOICE:
      enter_profile_options_state(result_data);
      break;
    case PROFILES_NEW: 
      ProfileEditorScreen::get_instance().new_profile();
      enter_profiles_state();
      break;
    case OPTION_BACK:
      enter_idle_state();
      break;
    case OPTION_NONE:
    default:
      break;
  }

  // // poll the EC11 object for the currently selected object
  // int choice = EC11::get_encoder_value();

  // Display::clear_buffers();
  // // print profiles
  // uint8_t num_profiles = Storage::read_all();
  // char temp_name_buffer[MAX_PROFILE_NAME_CHARS + 1];
  // for (int i = 0; i < num_profiles; ++i) {
  //   memset(temp_name_buffer, 0, sizeof(temp_name_buffer));
  //   Storage::read_profile_name(i, temp_name_buffer);
  //   Display::buff_text_println("%s %s", choice == i ? "[*]" : "[ ]", temp_name_buffer);
  // }

  // if (num_profiles == 0) {
  //   Display::buff_text_println("[ ] <no profiles>");
  // }
  // // print +profile option if necessary
  // if (num_profiles < 5) {
  //   Display::buff_text_println("%s [+]", choice == num_profiles ? "[*]" : "[ ]");
  // }
  // // print back option
  // Display::buff_text_println("%s Back", choice == min(num_profiles + 1, MAX_PROFILE_CHOICES) ? "[*]" : "[ ]");
  // Display::draw_text();

  // // select and change states depending on selection
  // if (EC11::poll_sw_event() == SW_PRESSED) {
  //   if (choice < num_profiles) {
  //     // chose an existing profile
  //     ProfileEditor editor;
  //     switch(editor.edit_profile(choice)) {
  //       case PROFILE_START: enter_reflow_state(choice); break;
  //       case PROFILE_CHANGED: 
  //         Storage::close(); // pref was set to read/write, end so we can restart it with just read
  //         enter_profiles_state();
  //         break;
  //       case PROFILE_NOTHING: 
  //       default: enter_profiles_state(); break;
  //     }
  //   } else if (choice == num_profiles) {
  //     // chose to add a new profile
  //     ProfileEditor editor;
  //     if (editor.new_profile()) {
  //       // pref was set to read/write, end so we can restart it with just read
  //       Storage::close();
  //     }
  //     // done with profile editor
  //     enter_profiles_state();
  //   } else {
  //     // back
  //     enter_idle_state();
  //   }
  // }
}

void profile_loop() {
  int result = ProfileScreen::get_instance().loop();
  int result_data = result & 0xf;
  result = result & 0xfff0;

  switch (result) {
    case PROFILE_OPTION_START:
      // TODO
      break;
    case PROFILE_OPTION_EDIT:
      ProfileEditorScreen::get_instance().edit_profile(result_data);
      enter_profile_options_state(result_data);
      break;
    case PROFILE_OPTION_DELETE:
      if (Storage::delete_profile(result_data)) {
        enter_profiles_state();
      }
      break;
    case OPTION_BACK:
      enter_profiles_state();
      break;
    case OPTION_NONE:
    default:
      break;
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
  switch (ManualScreen::get_instance().loop()) {
    case OPTION_BACK: enter_idle_state(); break;
    case OPTION_NONE:
    default: break;
  }
}

void debug_loop() {
  switch (DebugScreen::get_instance().loop()) {
    case OPTION_BACK: enter_idle_state(); break;
    case OPTION_NONE:
    default: break;
  }
}

void loop() {
  switch(oven_state) {
    case STATE_IDLE:
      idle_loop();
      break;
    case STATE_PROFILES_LIST:
      profiles_list_loop();
      break;
    case STATE_PROFILE_OPTIONS:
      profile_loop();
      break;
    case STATE_REFLOWING:
      reflow_loop();
      break;
    case STATE_MANUAL:
      manual_loop();
      break;
    case STATE_DEBUG:
    default:
      debug_loop();
      break;
  }
  delay(50);
}