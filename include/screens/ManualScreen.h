#ifndef MANUAL_SCREEN_H
#define MANUAL_SCREEN_H

#include "screens/ScreenStates.h"
#include "core/EC11.h"
#include "core/Display.h"
#include "core/Storage.h"
#include "core/TempSensor.h"

// singleton screen
class ManualScreen {
public:
    // make assignment and copy inaccessible
    ManualScreen(ManualScreen const&) = delete;
    ManualScreen(ManualScreen &&) = delete; 
    ManualScreen& operator=(ManualScreen const&) = delete;
    ManualScreen& operator=(ManualScreen &&) = delete;

    // get instance
    static ManualScreen& get_instance();

    void start();
    int loop();

private:
    ManualScreen();
    static float target_temp;

};

#endif