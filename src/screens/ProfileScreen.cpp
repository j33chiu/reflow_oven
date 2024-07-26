#include "screens/ProfileScreen.h"

uint8_t ProfileScreen::profile_idx = MAX_PROFILE_CHOICES;
char ProfileScreen::profile_name[MAX_PROFILE_NAME_CHARS + 1] = {'\0'};
int ProfileScreen::options_map[PROFILE_OPTIONS] = {
    PROFILE_OPTION_START, PROFILE_OPTION_EDIT, PROFILE_OPTION_DELETE, OPTION_BACK
};

ProfileScreen& ProfileScreen::get_instance() {
    static ProfileScreen instance;
    return instance;
}

ProfileScreen::ProfileScreen() {
    memset(profile_name, ' ', MAX_PROFILE_NAME_CHARS);  // set data in name to be all spaces
    profile_name[MAX_PROFILE_NAME_CHARS] = '\0';        // set null terminator at the end of the name
}

void ProfileScreen::start(const int profile_idx) {
    // start with profile idx
    if (profile_idx < MAX_PROFILE_CHOICES) {
        ProfileScreen::profile_idx = profile_idx;
        memset(profile_name, 0, sizeof(profile_name));
        Storage::read_profile_name(profile_idx, profile_name);
    }

    // setup ec11 choices: Start, Edit, Delete, Back (4 choices)
    EC11::set_rotary_options(PROFILE_OPTIONS);
    EC11::set_encoder_value(0);

    // print all options for the profile (start, edit, delete, back)
    Display::clear_buffers();
    Display::buff_text_println(profile_name);
    Display::buff_text_println("[*] Start");
    Display::buff_text_println("[ ] Edit");
    Display::buff_text_println("[ ] Delete");
    Display::buff_text_println("[ ] Back");
    Display::draw_text();
}

int ProfileScreen::loop() {
    // get knob choice
    int choice = EC11::get_encoder_value();
    choice = min(choice, PROFILE_OPTIONS - 1);
    choice = max(0, choice);

    // poll knob, if turned, update screen
    if (EC11::poll_kb_event() != KB_NONE) {
        // print all options for the profile (start, edit, delete, back)
        Display::clear_buffers();
        Display::buff_text_println(profile_name);
        Display::buff_text_println("%s Start", choice == 0 ? "[*]" : "[ ]");
        Display::buff_text_println("%s Edit", choice == 1 ? "[*]" : "[ ]");
        Display::buff_text_println("%s Delete", choice == 2 ? "[*]" : "[ ]");
        Display::buff_text_println("%s Back", choice == 3 ? "[*]" : "[ ]");
        Display::draw_text();
    }

    // poll knob, if pressed, perform selection
    if (EC11::poll_sw_event() == SW_PRESSED) {
        return options_map[choice] | profile_idx;
    }
    return OPTION_NONE;
}