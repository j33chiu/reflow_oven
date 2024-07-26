#include "screens/ProfileListScreen.h"

ProfileListScreen& ProfileListScreen::get_instance() {
    static ProfileListScreen instance;
    return instance;
}

ProfileListScreen::ProfileListScreen() {

}

void ProfileListScreen::start() {
    // get number of profiles
    uint8_t num_profiles = Storage::read_all();
    //                      // profiles    //back option    //new profile option
    EC11::set_rotary_options(num_profiles + 1 + (num_profiles == 5 ? 0 : 1));
    EC11::set_encoder_value(0);

    // print profiles
    int choice = EC11::get_encoder_value();
    Display::clear_buffers();
    char temp_name_buffer[MAX_PROFILE_NAME_CHARS + 1];
    for (int i = 0; i < num_profiles; ++i) {
        memset(temp_name_buffer, 0, sizeof(temp_name_buffer));
        Storage::read_profile_name(i, temp_name_buffer);
        Display::buff_text_println("%s %s", choice == i ? "[*]" : "[ ]", temp_name_buffer);
    }

    if (num_profiles == 0) {
        Display::buff_text_println("[ ] <no profiles>");
    }
    // print +profile option if necessary
    if (num_profiles < 5) {
        Display::buff_text_println("%s [+]", choice == num_profiles ? "[*]" : "[ ]");
    }
    // print back option
    Display::buff_text_println("%s Back", choice == min(num_profiles + 1, MAX_PROFILE_CHOICES) ? "[*]" : "[ ]");
    Display::draw_text();
}

int ProfileListScreen::loop() {
    // poll the EC11 object for the currently selected object
    int choice = EC11::get_encoder_value();
    uint8_t num_profiles = Storage::read_all();

    // poll the knob, if turned, update the screen
    if (EC11::poll_kb_event() != KB_NONE) {
        Display::clear_buffers();
        // print profiles
        char temp_name_buffer[MAX_PROFILE_NAME_CHARS + 1];
        for (int i = 0; i < num_profiles; ++i) {
            memset(temp_name_buffer, 0, sizeof(temp_name_buffer));
            Storage::read_profile_name(i, temp_name_buffer);
            Display::buff_text_println("%s %s", choice == i ? "[*]" : "[ ]", temp_name_buffer);
        }

        if (num_profiles == 0) {
            Display::buff_text_println("[ ] <no profiles>");
        }
        // print +profile option if necessary
        if (num_profiles < 5) {
            Display::buff_text_println("%s [+]", choice == num_profiles ? "[*]" : "[ ]");
        }
        // print back option
        Display::buff_text_println("%s Back", choice == min(num_profiles + 1, MAX_PROFILE_CHOICES) ? "[*]" : "[ ]");
        Display::draw_text();
    }

    // poll knob, if pressed, return selection
    if (EC11::poll_sw_event() == SW_PRESSED) {
        if (choice < num_profiles) {
            return PROFILES_CHOICE | choice;
        } else if (choice == num_profiles) {
            return PROFILES_NEW;
        } else {
            return OPTION_BACK;
        }
    }
    return OPTION_NONE;
}