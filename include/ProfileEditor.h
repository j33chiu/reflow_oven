#ifndef PROFILE_EDITOR_H
#define PROFILE_EDITOR_H

#include <Preferences.h>
#include "Display.h"
#include "EC11.h"

#define PREF_NAMESPACE "pref_rf_oven"
#define MAX_PROFILE_CHOICES     5                             // max profiles that can be saved
#define MAX_PROFILE_NAME_CHARS  16                            // max number of chars that can be saved for a profile name
#define MAX_PROFILE_DATA_POINTS 32                            // max number of nodes
#define MAX_PROFILE_DATA_CHARS  (MAX_PROFILE_DATA_POINTS * 8) // max number of chars that can be saved for a profile's data (timing and temp data), each node takes 8 chars to store, 
#define NUM_VALID_CHARS         37                            // number of valid characters for names (A-Z, 0-9, ' ')

// states for the naming screen
#define CURSOR_STATE        0       // cursor state for the naming screen, indicates scrolling through which character and screen options
#define ALPHA_NUM_STATE     1       // alpha-numeric state for the naming screen, indicates cycling through the character possibilities.

// states for the profile screen
#define TIME_STATE          0       // cursor state for setting the time of a node in the profile
#define TEMP_STATE          1       // temperature state for setting the temperature of a node in the profile

#define MAX_TIME        410     // max 400 seconds for the whole profile, 410 to allow setting the 400th second with the 10s resolution of the knob
#define TIME_INTERVALS  10      // amount of time each knob turn increments/decrements by
#define MAX_TEMP        265     // max 260 C for oven (for Panasonic oven, this is max that the original is rated at), 265 to allow setting 260 with the 5C resolution of knob
#define TEMP_INTERVALS  5       // temperature each knob turn increments/decrements by

// states for the existing profile screen
#define NUM_PROFILE_OPTIONS     4     // start, edit, delete, back

// results of choosing profile
#define PROFILE_START   0
#define PROFILE_CHANGED 1
#define PROFILE_NOTHING 2

// access profile info from preferences
const char profile_keys[MAX_PROFILE_CHOICES * 2][3] = { // 'n' is the profile name, 'p' is the actual profile data
  "n1", "p1",
  "n2", "p2",
  "n3", "p3",
  "n4", "p4",
  "n5", "p5",
};

// for easy cycling with knob
const char alpha_num[NUM_VALID_CHARS + 1] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

class ProfileEditor {

public:
    ProfileEditor(Preferences* pref, Display* display, EC11* ec11);
    
    bool new_profile();
    uint8_t edit_profile(int profile_num);


private:
    Preferences* pref = nullptr;
    Display* display = nullptr;
    EC11* ec11 = nullptr;

    int profile_idx = -1;

    char profile_name[MAX_PROFILE_NAME_CHARS + 1];
    char profile_data[MAX_PROFILE_DATA_CHARS + 1];


    bool name_screen();
    bool profile_screen();

    // constants
    const static uint8_t time_value_options = MAX_TIME / TIME_INTERVALS;
    const static uint8_t time_state_options = time_value_options + 2; // time intervals + done and back options
    const static uint8_t temp_value_options = MAX_TEMP / TEMP_INTERVALS;

    uint8_t profile_state[time_state_options] = {0};

};

#endif