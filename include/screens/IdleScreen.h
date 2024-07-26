#ifndef IDLE_SCREEN_H
#define IDLE_SCREEN_H

#include "screens/ScreenStates.h"
#include "core/EC11.h"
#include "core/Display.h"
#include "core/Storage.h"

// singleton screen
class IdleScreen {
public:
    // make assignment and copy inaccessible
    IdleScreen(IdleScreen const&) = delete;
    IdleScreen(IdleScreen &&) = delete; 
    IdleScreen& operator=(IdleScreen const&) = delete;
    IdleScreen& operator=(IdleScreen &&) = delete;

    // get instance
    static IdleScreen& get_instance();

    void start();
    int loop();

private:
    IdleScreen();
    static int options_map[IDLE_OPTIONS];

};

#endif