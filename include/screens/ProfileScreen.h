#ifndef PROFILE_SCREEN_H
#define PROFILE_SCREEN_H

#include "screens/ScreenStates.h"
#include "core/EC11.h"
#include "core/Display.h"
#include "core/Storage.h"

// singleton screen
class ProfileScreen {
public:
    // make assignment and copy inaccessible
    ProfileScreen(ProfileScreen const&) = delete;
    ProfileScreen(ProfileScreen &&) = delete; 
    ProfileScreen& operator=(ProfileScreen const&) = delete;
    ProfileScreen& operator=(ProfileScreen &&) = delete;

    // get instance
    static ProfileScreen& get_instance();

    void start(const int profile_idx = -1);
    int loop();

private:
    ProfileScreen();
    static uint8_t profile_idx;
    static char profile_name[MAX_PROFILE_NAME_CHARS + 1];
    static int options_map[PROFILE_OPTIONS];

};

#endif