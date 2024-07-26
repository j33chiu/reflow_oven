#ifndef PROFILE_LIST_SCREEN_H
#define PROFILE_LIST_SCREEN_H

#include "screens/ScreenStates.h"
#include "core/EC11.h"
#include "core/Display.h"
#include "core/Storage.h"

// singleton screen
class ProfileListScreen {
public:
    // make assignment and copy inaccessible
    ProfileListScreen(ProfileListScreen const&) = delete;
    ProfileListScreen(ProfileListScreen &&) = delete; 
    ProfileListScreen& operator=(ProfileListScreen const&) = delete;
    ProfileListScreen& operator=(ProfileListScreen &&) = delete;

    // get instance
    static ProfileListScreen& get_instance();

    void start();
    int loop();

private:
    ProfileListScreen();

};

#endif