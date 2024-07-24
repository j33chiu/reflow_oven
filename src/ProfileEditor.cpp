#include "ProfileEditor.h"

ProfileEditor::ProfileEditor(Preferences* pref) {
    this->pref = pref;
    memset(profile_name, ' ', MAX_PROFILE_NAME_CHARS);          // reset data in name
    profile_name[MAX_PROFILE_NAME_CHARS] = '\0';
    memset(profile_state, 0, time_state_options*sizeof(int));   // reset profile_state to zeros
}

bool ProfileEditor::new_profile() {
    // count currently existing profiles
    int num_existing = 0;
    for (int i = 0; i < MAX_PROFILE_CHOICES; i++) {
        if (!pref->isKey(profile_keys[i * 2])) break;   // key is not valid
        ++num_existing; // key is valid, profile exists
    }
    // if there are the max number of profiles, exit
    if (num_existing == MAX_PROFILE_CHOICES) {
        return false;
    }
    // current profile index (0-indexed) is set to the number of currently existing profiles
    profile_idx = num_existing;

    // profile creation
    if (!this->name_screen()) {
        // user pressed 'back', exit profile creation without creating a profile
        return false;
    } else {
        // properly made profile, save data
        pref->end();
        pref->begin(PREF_NAMESPACE, false);
        pref->putString(profile_keys[profile_idx * 2], profile_name);   // save the profile name
        pref->putString(profile_keys[(profile_idx * 2) + 1], profile_data);   // save the profile data
    }
    return true;
}

uint8_t ProfileEditor::edit_profile(int profile_idx) {
    // current profile index (0-indexed) is set
    this->profile_idx = profile_idx;
    // get data of the profile from preferences
    pref->getString(profile_keys[profile_idx * 2], profile_name, MAX_PROFILE_NAME_CHARS + 1);
    pref->getString(profile_keys[(profile_idx * 2) + 1], profile_data, MAX_PROFILE_DATA_CHARS + 1);
    
    // setup ec11 choices: Start, Edit, Delete, Back (4 choices)
    EC11::set_rotary_options(NUM_PROFILE_OPTIONS);
    EC11::set_encoder_value(0);

    uint8_t output_state = PROFILE_NOTHING;
    bool chose_option = false;
    while (!chose_option) {
        int choice = EC11::get_encoder_value();

        // print all options for the profile (start, edit, delete, back)
        Display::clear_buffers();
        Display::buff_text_println(profile_name);
        Display::buff_text_println("%s Start", choice == 0 ? "[*]" : "[ ]");
        Display::buff_text_println("%s Edit", choice == 1 ? "[*]" : "[ ]");
        Display::buff_text_println("%s Delete", choice == 2 ? "[*]" : "[ ]");
        Display::buff_text_println("%s Back", choice == 3 ? "[*]" : "[ ]");
        Display::draw_text();

        // select and change states depending on selection
        if (EC11::poll_sw_event() == SW_PRESSED) {
            chose_option = true;
            if (choice == 0) {
                // chose "start", exit and return state
                output_state = PROFILE_START;
            } else if (choice == 1) {
                // chose "edit"        
                // setup profile_state, then switch to name and profile edit screens
                for (int profile_ptr = 0; profile_ptr < MAX_PROFILE_DATA_CHARS; profile_ptr += 4) {
                    int time = atoi(profile_data + profile_ptr);
                    profile_ptr += 4;
                    int temp = atoi(profile_data + profile_ptr);
                    profile_state[time / TIME_INTERVALS] = temp / TEMP_INTERVALS;
                }
                // enter the naming and profile screens after setting the current profile data already.
                if (this->name_screen()) {
                    // properly edited profile, save data
                    pref->end();
                    pref->begin(PREF_NAMESPACE, false);
                    pref->putString(profile_keys[profile_idx * 2], profile_name);   // save the profile name
                    pref->putString(profile_keys[(profile_idx * 2) + 1], profile_data);   // save the profile data
                    output_state = PROFILE_CHANGED;
                } else {
                    // if here, user clicked "back", exiting the editing screens and we go back to the screen displaying all the options for the profile
                    chose_option = false;
                }
            } else if (choice == 2) {
                // chose "delete"
                pref->end();
                pref->begin(PREF_NAMESPACE, false);
                // shift profiles (logic: deleting profile in slot 2 should shift all subsequent slots. can do this by shifting everything, then deleting whats at the end)
                int i = profile_idx;
                for (i = profile_idx + 1; i < MAX_PROFILE_CHOICES; i++) {
                    if (pref->isKey(profile_keys[i * 2])) {
                        pref->putString(profile_keys[(i - 1) * 2], pref->getString(profile_keys[i * 2]));
                        pref->putString(profile_keys[((i - 1) * 2) + 1], pref->getString(profile_keys[(i * 2) + 1]));
                    } else {
                        break;
                    }
                }
                // remove last profile from pref after shift
                --i;
                pref->remove(profile_keys[i * 2]);
                pref->remove(profile_keys[(i * 2) + 1]);
                output_state = PROFILE_CHANGED;
            } else {
                // chose "back", return nothing
                output_state = PROFILE_NOTHING;
            }
        }
    }
    return output_state;
}

bool ProfileEditor::name_screen() {
    // setup ec11 choices (number of characters that can be edited + 2)
    EC11::set_rotary_options(MAX_PROFILE_NAME_CHARS + 2); // allow rotating between all chars and the "done" and "back" choices
    EC11::set_encoder_value(0);

    // Cursor state goes between the character positions and the "done"/"back" options, ALPHA_NUM_STATE is after selecting a char position and goes between letters/numbers
    int state = CURSOR_STATE;

    bool done_name = false;
    int cursor_position = EC11::get_encoder_value();
    int char_choice = 0;

    // while loop to update ui
    while (!done_name) {
        // current option of knob
        if (state == CURSOR_STATE) {
            cursor_position = EC11::get_encoder_value();
        } else if (state == ALPHA_NUM_STATE) {
            char_choice = EC11::get_encoder_value();
            profile_name[cursor_position] = alpha_num[char_choice];
        }

        // display name and options
        Display::clear_buffers();
        Display::buff_text_println("Name:");
        Display::buff_text_println(profile_name);
        for (int i = 0; i < MAX_PROFILE_NAME_CHARS; i++) {
            if (i == cursor_position) {
                Display::buff_text_print("^");
            } else {
                Display::buff_text_print(" ");
            }
        }
        Display::buff_text_endl();
        Display::buff_text_println("%s Done", cursor_position == MAX_PROFILE_NAME_CHARS ? "[*]" : "[ ]");
        Display::buff_text_println("%s Back", cursor_position == (MAX_PROFILE_NAME_CHARS + 1) ? "[*]" : "[ ]");
        Display::draw_text();

        // select and change states depending on selection
        if (EC11::poll_sw_event() == SW_PRESSED) {
            if (cursor_position < MAX_PROFILE_NAME_CHARS) {
                // selected a character position
                if (state == CURSOR_STATE) {
                    // transition to edit the char state
                    state = ALPHA_NUM_STATE;
                    // set the encoder value to the character already there
                    EC11::set_rotary_options(NUM_VALID_CHARS);
                    char current_char = profile_name[cursor_position];
                    int start_encoder_value = 0;
                    for (int i = 0; i < NUM_VALID_CHARS; i++) {
                        if (alpha_num[i] == current_char) {
                            start_encoder_value = i;
                            break;
                        }
                    }
                    EC11::set_encoder_value(start_encoder_value);
                } else if (state == ALPHA_NUM_STATE) {
                    // transition to cursor state
                    state = CURSOR_STATE;
                    // done setting the current character, revert ec11 back to cursor mode
                    EC11::set_rotary_options(MAX_PROFILE_NAME_CHARS + 2);
                    // set cursor value to the current cursor position
                    EC11::set_encoder_value(cursor_position);
                }
            } else if (cursor_position == MAX_PROFILE_NAME_CHARS) {
                // chose "done"
                // move onto profile screen
                if (!profile_screen()) {
                    // chose to come back to this screen
                    // revert ec11 back to cursor mode
                    EC11::set_rotary_options(MAX_PROFILE_NAME_CHARS + 2);
                    EC11::set_encoder_value(cursor_position);
                } else {
                    // done with profile, can exit
                    done_name = true;
                }
            } else {
                // "back" selection
                return false;
            }
        }
    }
    return true;
}

bool ProfileEditor::profile_screen() {
    // time in intervals of 10s
    // temperature in intervals of 5C


    // setup ec11 choices
    EC11::set_rotary_options(time_state_options);
    EC11::set_encoder_value(0);

    // setup display graph limits
    Display::set_graph_lim(0, MAX_TIME - TIME_INTERVALS, 0, MAX_TEMP - TEMP_INTERVALS);

    int state = TIME_STATE;

    bool done_profile = false;
    int time_position = EC11::get_encoder_value();
    int time_value = time_position * TIME_INTERVALS;
    int temp_position = 0;
    int temp_value = temp_position * TEMP_INTERVALS;

    // while loop to update ui
    while (!done_profile) {
        // clear display
        Display::clear_buffers();

        // draw graph
        // depending on the state, we dynamically draw a tick (indicating time setting) or x (indicating temp setting) to correspond to user selections.
        if (state == TIME_STATE) {
            // in time state, where user chooses a time at which to add a temperature node/target
            // update time knob position and value
            time_position = EC11::get_encoder_value();
            time_value = min(time_position * TIME_INTERVALS, MAX_TIME - TIME_INTERVALS);
            // as the time cursor moves across the graph, update the temperature accordingly
            temp_value = profile_state[time_position] * TEMP_INTERVALS;
            // draw a vertical bar indicating cursor position
            if (time_position < time_value_options) Display::buff_graph_tick(time_value, true); // only draw if not selecting the "back" or "done" options
        } else if (state == TEMP_STATE) {
            // in temp state, where user has chosen a time and is currently adjusting the target temperature
            // update temperature knob position and value
            temp_position = EC11::get_encoder_value();
            temp_value = temp_position * TEMP_INTERVALS;
            // draw an 'x' corresponding to the current temperature being set
            Display::buff_graph_point(time_value, temp_value);
        }
        // display time and temp on the graph axes unless in the "done" or "back" options
        if (time_position < time_value_options) {
            Display::set_graph_axes(time_value, temp_value);
        } else {
            Display::set_graph_axes("   ", "   ");
        }
        // draw the profile on the graph
        // for every non-zero point in the profile, draw a point and count how many nodes exist
        int profile_nodes = 0;
        for (int i = 0; i < time_value_options; i++) {
            if (profile_state[i] != 0) {
                ++profile_nodes;
                Display::buff_graph_point(i * TIME_INTERVALS, profile_state[i] * ((int)TEMP_INTERVALS));
            }
        }
        // display 'done' and 'back' options
        Display::buff_text_print("%s Done   ", time_position == time_value_options ? "[*]" : "[ ]");
        Display::buff_text_println("%s Back", time_position == (time_value_options + 1) ? "[*]" : "[ ]");
        // display the text (done and back options)
        Display::draw_text(0, SCREEN_HEIGHT - (CHAR_PX_HEIGHT + 2) + 2, false); // don't draw to the screen yet since this causes flickering
        // display the graph
        Display::draw_graph();

        // select and change states depending on selection
        if (EC11::poll_sw_event() == SW_PRESSED) {
            if (time_position < time_value_options) {
                // selected a time on the graph
                if (state == TIME_STATE) {
                    // start editing temperature if not too many nodes. If already at the limit, only allow editing of temperature if it is an existing node (non-0)
                    if (temp_value != 0 || profile_nodes < MAX_PROFILE_DATA_POINTS) {
                        // transition to temperature edit state
                        EC11::set_rotary_options(temp_value_options);
                        EC11::set_encoder_value(temp_value / TEMP_INTERVALS);
                        state = TEMP_STATE;
                    }
                } else if (state == TEMP_STATE) {
                    // set the node value and go back to the time state
                    profile_state[time_position] = temp_position;
                    // transition back to time edit state
                    EC11::set_rotary_options(time_state_options);
                    EC11::set_encoder_value(time_value / TIME_INTERVALS);
                    state = TIME_STATE;
                }
            } else if (time_position == time_value_options) {
                // chose "done"
                if (profile_idx < 0) return false; // error

                // loop through profile state array. any non-0 values are added to the profile_data string
                int profile_data_ptr_pos = 0;
                for (int i = 0; i < time_value_options; i++) {
                    if (profile_state[i] != 0) {
                        if (profile_data_ptr_pos >= 0 && profile_data_ptr_pos < MAX_PROFILE_DATA_CHARS + 1) {
                            profile_data_ptr_pos += snprintf(profile_data + profile_data_ptr_pos, MAX_PROFILE_DATA_CHARS + 1 - profile_data_ptr_pos, "%.3d,%.3d,", i * TIME_INTERVALS, profile_state[i] * (int)TEMP_INTERVALS);
                        }
                    }
                }
                done_profile = true;
            } else {
                // chose "back"
                return false;
            }
        }
    }
    return true;
}