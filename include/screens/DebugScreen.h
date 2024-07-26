#ifndef DEBUG_SCREEN_H
#define DEBUG_SCREEN_H

#include "screens/ScreenStates.h"
#include "core/EC11.h"
#include "core/Display.h"
#include "core/Storage.h"
#include "core/TempSensor.h"

// singleton screen
class DebugScreen {
public:
    // make assignment and copy inaccessible
    DebugScreen(DebugScreen const&) = delete;
    DebugScreen(DebugScreen &&) = delete; 
    DebugScreen& operator=(DebugScreen const&) = delete;
    DebugScreen& operator=(DebugScreen &&) = delete;

    // get instance
    static DebugScreen& get_instance();

    void start();
    int loop();

private:
    DebugScreen();
    static float temp_value;

};

#endif