#ifndef PROFILE_EDITOR_SCREEN_H
#define PROFILE_EDITOR_SCREEN_H

#include "core/EC11.h"
#include "core/Storage.h"
#include "core/Display.h"
#include "screens/ScreenStates.h"

#define NUM_VALID_CHARS     37      // number of valid characters for names (A-Z, 0-9, ' ')

#define MAX_TIME        410     // max 400 seconds for the whole profile, 410 to allow setting the 400th second with the 10s resolution of the knob
#define TIME_INTERVALS  10      // amount of time each knob turn increments/decrements by
#define MAX_TEMP        265     // max 260 C for oven (for Panasonic oven, this is max that the original is rated at), 265 to allow setting 260 with the 5C resolution of knob
#define TEMP_INTERVALS  5       // temperature each knob turn increments/decrements by

class ProfileEditorScreen {
public:
    // make assignment and copy inaccessible
    ProfileEditorScreen(ProfileEditorScreen const&) = delete;
    ProfileEditorScreen(ProfileEditorScreen &&) = delete; 
    ProfileEditorScreen& operator=(ProfileEditorScreen const&) = delete;
    ProfileEditorScreen& operator=(ProfileEditorScreen &&) = delete;

    // get instance
    static ProfileEditorScreen& get_instance();

    uint16_t new_profile();
    uint16_t edit_profile(const uint8_t profile_idx = MAX_PROFILE_CHOICES);

private:
    // constants
    static const uint8_t time_value_options = MAX_TIME / TIME_INTERVALS;
    static const uint8_t time_state_options = time_value_options + 2; // time intervals + done and back options
    static const uint8_t temp_value_options = MAX_TEMP / TEMP_INTERVALS;
    static const char alpha_num[NUM_VALID_CHARS + 1];

    ProfileEditorScreen();

    // screen state variables
    static uint8_t profile_idx;
    static uint16_t screen_state;
    static uint16_t knob_state;

    // screen data variables
    static int cursor_position;
    static int time_value;
    static int num_profile_nodes;

    static char profile_name[MAX_PROFILE_NAME_CHARS + 1];
    static char profile_data[MAX_PROFILE_DATA_CHARS + 1];
    static uint8_t profile_state[time_state_options];

    static void enter_name_screen();
    static void enter_data_screen();

    static void swap_knob_states();

    static uint16_t name_loop();
    static uint16_t data_loop();
    static uint16_t screen_loop();


};

#endif