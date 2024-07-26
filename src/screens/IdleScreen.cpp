#include "screens/IdleScreen.h"

int IdleScreen::options_map[IDLE_OPTIONS] = {
    IDLE_OPTION_PROFILES, IDLE_OPTION_MANUAL, IDLE_OPTION_DEBUG
};

IdleScreen& IdleScreen::get_instance() {
    static IdleScreen instance;
    return instance;
}

IdleScreen::IdleScreen() {

}

void IdleScreen::start() {
    Storage::close();   // in idle mode, preferences don't need to be open
    EC11::set_rotary_options(IDLE_OPTIONS);
    EC11::set_encoder_value(0);

    // display screen, start at choice 0 so can be hardcoded
    Display::clear_buffers();
    Display::buff_text_println("[*] Profiles");
    Display::buff_text_println("[ ] Manual Mode");
    Display::buff_text_println("[ ] Debug Mode");
    Display::draw_text();
}

int IdleScreen::loop() {
    // poll knob for currently selected option
    int choice = EC11::get_encoder_value();
    choice = min(choice, IDLE_OPTIONS - 1);
    choice = max(0, choice);

    // poll knob, if turned, update the screen
    if (EC11::poll_kb_event() != KB_NONE) {
        Display::clear_buffers();
        Display::buff_text_println("%s Profiles", choice == 0 ? "[*]" : "[ ]");
        Display::buff_text_println("%s Manual Mode", choice == 1 ? "[*]" : "[ ]");
        Display::buff_text_println("%s Debug Mode", choice == 2 ? "[*]" : "[ ]");
        Display::draw_text();
    }

    // poll knob, if pressed, return selection
    if (EC11::poll_sw_event() == SW_PRESSED) {
        return options_map[choice];
    }
    return OPTION_NONE;
}