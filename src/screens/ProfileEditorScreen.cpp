#include "screens/ProfileEditorScreen.h"

const char ProfileEditorScreen::alpha_num[NUM_VALID_CHARS + 1] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

uint8_t ProfileEditorScreen::profile_idx = MAX_PROFILE_CHOICES;
uint16_t ProfileEditorScreen::screen_state = 0;
uint16_t ProfileEditorScreen::knob_state = STATE_CURSOR;

int ProfileEditorScreen::cursor_position = 0;
int ProfileEditorScreen::time_value = 0;
int ProfileEditorScreen::num_profile_nodes = 0;

char ProfileEditorScreen::profile_name[MAX_PROFILE_NAME_CHARS + 1] = {'\0'};
char ProfileEditorScreen::profile_data[MAX_PROFILE_DATA_CHARS + 1] = {'\0'};
uint8_t ProfileEditorScreen::profile_state[time_state_options] = {0};

ProfileEditorScreen& ProfileEditorScreen::get_instance() {
    static ProfileEditorScreen instance;
    return instance;
}

ProfileEditorScreen::ProfileEditorScreen() {
}

uint16_t ProfileEditorScreen::new_profile() {
    // get number of currently existing profiles
    uint8_t num_existing = Storage::read_all();
    // if too many profiles already, error and return
    if (num_existing == MAX_PROFILE_CHOICES) {
        return PROFILE_EDITOR_ERROR;
    }
    // current new profile index (0-indexed) will be the number of existing profiles currently
    ProfileEditorScreen::profile_idx = num_existing;
    // reset profile name and data strings
    memset(profile_name, ' ', MAX_PROFILE_NAME_CHARS);              // set data in name to be all spaces
    profile_name[MAX_PROFILE_NAME_CHARS] = '\0';                    // set null terminator at the end of the name
    memset(profile_data, 0, sizeof(profile_data));                  // set data in profile to be all null
    memset(profile_state, 0, sizeof(uint8_t) * time_state_options); // set profile data to 0

    enter_name_screen();
    return screen_loop();
}

uint16_t ProfileEditorScreen::edit_profile(const uint8_t profile_idx) {
    // current profile index (0-indexed) is set
    ProfileEditorScreen::profile_idx = profile_idx;
    // reset current data buffers
    memset(profile_name, ' ', MAX_PROFILE_NAME_CHARS);              // set data in name to be all spaces
    profile_name[MAX_PROFILE_NAME_CHARS] = '\0';                    // set null terminator at the end of the name
    memset(profile_data, 0, sizeof(profile_data));                  // set data in profile to be all null
    memset(profile_state, 0, sizeof(uint8_t) * time_state_options); // set profile data to 0

    // get data of the profile from preferences
    Storage::read_profile(profile_idx, profile_name, profile_data);
    // read profile data string into profile_state array
    for (int profile_ptr = 0; profile_ptr < MAX_PROFILE_DATA_CHARS; profile_ptr += 4) {
        int time = atoi(profile_data + profile_ptr);
        profile_ptr += 4;
        int temp = atoi(profile_data + profile_ptr);
        profile_state[time / TIME_INTERVALS] = temp / TEMP_INTERVALS;
    }
    // reset data string
    memset(profile_data, 0, sizeof(profile_data));

    enter_name_screen();
    return screen_loop();
} 

void ProfileEditorScreen::enter_name_screen() {
    // setup ec11 choices (number of characters that can be edited + 2)
    EC11::set_rotary_options(MAX_PROFILE_NAME_CHARS + 2); // allow rotating between all chars and the "done" and "back" choices
    EC11::set_encoder_value(0);

    screen_state = STATE_NAME_EDITOR;
    knob_state = STATE_CURSOR;

    cursor_position = 0;

    Display::clear_buffers();
    Display::buff_text_println("Name:");
    Display::buff_text_println(profile_name);
    Display::buff_text_println("^");
    Display::buff_text_println("[ ] Done");
    Display::buff_text_println("[ ] Back");
    Display::draw_text();
}

void ProfileEditorScreen::enter_data_screen() {
    // reset cursor
    cursor_position = 0;
    time_value = cursor_position * TIME_INTERVALS;
    // setup ec11 choices
    EC11::set_rotary_options(time_state_options);
    EC11::set_encoder_value(cursor_position);
    // setup display graph limits
    Display::set_graph_lim(0, MAX_TIME - TIME_INTERVALS, 0, MAX_TEMP - TEMP_INTERVALS);

    // set states
    screen_state = STATE_CURVE_EDITOR;
    knob_state = STATE_CURSOR;

    // draw initial display
    // clear display
    Display::clear_buffers();
    // draw a tick on the x-axis at 0 (cursor position)
    Display::buff_graph_tick(time_value, true);
    // set axes labels time and temp values
    Display::set_graph_axes(time_value, profile_state[0] * TEMP_INTERVALS);
    // count number of nodes and display each on the graph
    num_profile_nodes = 0;
    for (int i = 0; i < time_value_options; i++) {
        if (profile_state[i] != 0) {
            ++num_profile_nodes;
            Display::buff_graph_point(i * TIME_INTERVALS, profile_state[i] * ((int)TEMP_INTERVALS));
        }
    }
    // draw done and back options at the bottom of the screen
    Display::buff_text_print("[ ] Done   ");
    Display::buff_text_println("[ ] Back");
    Display::draw_text(0, SCREEN_HEIGHT - (CHAR_PX_HEIGHT + 2) + 2, false); // don't draw to the screen yet since this causes flickering
    // display the graph
    Display::draw_graph();

}

void ProfileEditorScreen::swap_knob_states() {
    switch (knob_state | screen_state) {
        case STATE_CURSOR | STATE_NAME_EDITOR:
            // in name edit screen, cursor state
            // transition states
            knob_state = STATE_DATA_EDIT;
            // set the encoder value to the character already there
            EC11::set_rotary_options(NUM_VALID_CHARS);
            EC11::set_encoder_value((int)(strchr(alpha_num, profile_name[cursor_position]) - alpha_num));
            break;
        case STATE_CURSOR | STATE_CURVE_EDITOR:
            // in curve edit screen, cursor state
            // transition to temperature edit state if not too many nodes. if at the limit, only allow editing if it is an existing node (non-0)
            if (profile_state[cursor_position] != 0 || num_profile_nodes < MAX_PROFILE_DATA_POINTS) {
                EC11::set_rotary_options(temp_value_options);
                EC11::set_encoder_value(profile_state[cursor_position]);
                knob_state = STATE_DATA_EDIT;
            }
            break;
        case STATE_DATA_EDIT | STATE_NAME_EDITOR:
            // in name edit screen, editing letter state
            // transition states
            knob_state = STATE_CURSOR;
            // done setting the current character, revert ec11 back to cursor mode, pointing at the original position
            EC11::set_rotary_options(MAX_PROFILE_NAME_CHARS + 2);
            EC11::set_encoder_value(cursor_position);
            break;
        case STATE_DATA_EDIT | STATE_CURVE_EDITOR:
            // in curve edit screen, editing node value (temperature) state
            // transition states
            knob_state = STATE_CURSOR;
            // done setting the current node temperature, revert ec11 back to cursor mode, pointing at the original position
            EC11::set_rotary_options(time_state_options);
            EC11::set_encoder_value(cursor_position);
            break;
        default:
            break;
    }
}

uint16_t ProfileEditorScreen::name_loop() {
    // set cursor position or character choice depending on state
    switch (knob_state) {
        case STATE_CURSOR: cursor_position = EC11::get_encoder_value(); break;
        case STATE_DATA_EDIT: profile_name[cursor_position] = alpha_num[EC11::get_encoder_value()]; break;
        default: return PROFILE_EDITOR_ERROR;
    }

    // poll knob, if turned, update display
    //if (EC11::poll_kb_event() != KB_NONE) {
    Display::clear_buffers();
    Display::buff_text_println("Name:");
    Display::buff_text_println(profile_name);
    for (int i = 0; i < MAX_PROFILE_NAME_CHARS; ++i) {
        Display::buff_text_print(cursor_position == i ? "^" : " ");
    }
    Display::buff_text_endl();
    Display::buff_text_println("%s Done", cursor_position == MAX_PROFILE_NAME_CHARS ? "[*]" : "[ ]");
    Display::buff_text_println("%s Back", cursor_position == (MAX_PROFILE_NAME_CHARS + 1) ? "[*]" : "[ ]");
    Display::draw_text();
    //}

    // poll knob, if pressed, process selection
    if (EC11::poll_sw_event() == SW_PRESSED) {
        if (cursor_position < MAX_PROFILE_NAME_CHARS) {
            // chose a letter, need to swap knob states
            swap_knob_states();
        } else if (cursor_position == MAX_PROFILE_NAME_CHARS) {
            // chose "done"
            enter_data_screen();
        } else {
            // chose "back"
            return OPTION_BACK;
        }
    }

    return OPTION_NONE;
}

uint16_t ProfileEditorScreen::data_loop() {
    // set cursor position or temperature value depending on the state
    switch (knob_state) {
        case STATE_CURSOR: cursor_position = EC11::get_encoder_value(); break;
        case STATE_DATA_EDIT: profile_state[cursor_position] = EC11::get_encoder_value(); break;
        default: return PROFILE_EDITOR_ERROR;
    }
    time_value = min(cursor_position * TIME_INTERVALS, MAX_TIME - TIME_INTERVALS);
    int temperature_value = profile_state[cursor_position] * ((int)TEMP_INTERVALS);

    // poll knob, if turned, update display
    //if (EC11::poll_kb_event() != KB_NONE) {
    // clear display
    Display::clear_buffers();
    // if in cursor state, draw a tick on the x axis
    if (knob_state == STATE_CURSOR && cursor_position < time_value_options) Display::buff_graph_tick(time_value, true);
    // display time and temp labels on the graph unless in the "done" or "back" options
    if (cursor_position < time_value_options) Display::set_graph_axes(time_value, temperature_value);
    else Display::set_graph_axes("   ", "   ");
    // count number of nodes and display each on the graph
    num_profile_nodes = 0;
    for (int i = 0; i < time_value_options; i++) {
        if (profile_state[i] != 0) {
            ++num_profile_nodes;
            Display::buff_graph_point(i * TIME_INTERVALS, profile_state[i] * ((int)TEMP_INTERVALS));
        }
    }
    // display 'done' and 'back' options
    Display::buff_text_print("%s Done   ", cursor_position == time_value_options ? "[*]" : "[ ]");
    Display::buff_text_println("%s Back", cursor_position == (time_value_options + 1) ? "[*]" : "[ ]");
    // display the text (done and back options)
    Display::draw_text(0, SCREEN_HEIGHT - (CHAR_PX_HEIGHT + 2) + 2, false); // don't draw to the screen yet since this causes flickering
    // display the graph
    Display::draw_graph();
    //}

    // poll knob, if pressed, process selection
    if (EC11::poll_sw_event() == SW_PRESSED) {
        if (cursor_position < time_value_options) {
            // selected a time on the graph, need to swap states
            swap_knob_states();
        } else if (cursor_position == time_value_options) {
            // chose "done"
            // loop through profile state array. any non-0 values are added to the profile_data string
            int profile_data_ptr_pos = 0;
            for (int i = 0; i < time_value_options; i++) {
                if (profile_state[i] != 0) {
                    if (profile_data_ptr_pos >= 0 && profile_data_ptr_pos < MAX_PROFILE_DATA_CHARS + 1) {
                        profile_data_ptr_pos += snprintf(profile_data + profile_data_ptr_pos, MAX_PROFILE_DATA_CHARS + 1 - profile_data_ptr_pos, "%.3d,%.3d,", i * TIME_INTERVALS, profile_state[i] * (int)TEMP_INTERVALS);
                    }
                }
            }
            Storage::write_profile(profile_idx, profile_name, profile_data);
            return PROFILE_EDITOR_DONE;
        } else {
            // chose "back"
            enter_name_screen();
        }
    }

    return OPTION_NONE;
}

uint16_t ProfileEditorScreen::screen_loop() {
    uint16_t result = OPTION_NONE;
    while (result == OPTION_NONE) {
        switch (screen_state) {
            case STATE_NAME_EDITOR: result = name_loop(); break;
            case STATE_CURVE_EDITOR: result = data_loop(); break;
            default: result = PROFILE_EDITOR_ERROR; break;
        }
    }
    return result;
}