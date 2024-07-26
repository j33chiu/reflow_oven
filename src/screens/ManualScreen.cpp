#include "screens/ManualScreen.h"

float ManualScreen::target_temp = 0.0f;

ManualScreen& ManualScreen::get_instance() {
    static ManualScreen instance;
    return instance;
}

ManualScreen::ManualScreen() {
    TempSensor::get_instance().update();
    if (TempSensor::get_instance().getNumDevices() == 0) {
        Display::clear_buffers();
        Display::buff_text_println("No Temperature Sensors Found...");
        Display::draw_text();
    } else {
        // get address of sensor 0
        target_temp = TempSensor::get_instance().getTempCByIndex(0);
    }
}

void ManualScreen::start() {
    Storage::close();   // in manual mode, preferences don't need to be open
    // set encoder to be limitless on value
    EC11::set_rotary_options(-1);
    EC11::set_encoder_value(target_temp); 

    // display screen
    Display::clear_buffers();
    Display::buff_text_println("Target Temp: %06.2f C", target_temp);
    Display::buff_text_println("\nClick knob to exit");
    Display::draw_text();
}

int ManualScreen::loop() {
    // poll knob for current value
    target_temp = EC11::get_encoder_value();

    // poll knob, if turned, update the screen
    if (EC11::poll_kb_event() != KB_NONE) {
        Display::clear_buffers();
        Display::buff_text_println("Target Temp: %06.2f C", target_temp);
        Display::buff_text_println("\nClick knob to exit");
        Display::draw_text();
    }

    // poll knob, if pressed, return selection
    if (EC11::poll_sw_event() == SW_PRESSED) {
        return OPTION_BACK;
    }
    return OPTION_NONE;
}