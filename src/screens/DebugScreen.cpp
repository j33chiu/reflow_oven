#include "screens/DebugScreen.h"

float DebugScreen::temp_value = 0.0f;

DebugScreen& DebugScreen::get_instance() {
    static DebugScreen instance;
    return instance;
}

DebugScreen::DebugScreen() {
}

void DebugScreen::start() {
    Storage::close();   // in debug mode, preferences don't need to be open
    EC11::set_rotary_options(-1); 

    TempSensor::get_instance().update();

    Display::clear_buffers();
    if (TempSensor::get_instance().getNumDevices() == 0) {
        Display::buff_text_println("No Temperature Sensors Found...");
    } else {
        // get address of sensor 0
        temp_value = TempSensor::get_instance().getTempCByIndex(0);
        Display::buff_text_println("Temperature: %06.2f", temp_value);
    }
    Display::buff_text_println("\nClick knob to exit");
    Display::draw_text();
}

int DebugScreen::loop() {
    // get temperature from sensor
    TempSensor::get_instance().update();
    float compare = temp_value;
    if (TempSensor::get_instance().getNumDevices() != 0) {
        temp_value = TempSensor::get_instance().getTempCByIndex(0);
    }
    // if temperature changed, print
    if (compare != temp_value) {
        Display::clear_buffers();
        Display::buff_text_println("Temperature: %06.2f", temp_value);
        Display::buff_text_println("\nClick knob to exit");
        Display::draw_text();
    }

    // if clicked, return to idle state
    if (EC11::poll_sw_event() == SW_PRESSED) {
        return OPTION_BACK;
    }
    return OPTION_NONE;
}