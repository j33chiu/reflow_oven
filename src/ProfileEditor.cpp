#include "ProfileEditor.h"

ProfileEditor::ProfileEditor(Preferences* pref, Display* display, EC11* ec11) {
    this->pref = pref;
    this->display = display;
    this->ec11 = ec11;
    memset(profile_name, ' ', MAX_PROFILE_NAME_CHARS);      // reset data in name
    profile_name[MAX_PROFILE_NAME_CHARS] = '\0';
    memset(profile_state, 0, time_state_options*sizeof(int));           // reset profile_state to zeros
}

bool ProfileEditor::new_profile() {
    int num_existing = 0;
    for (int i = 0; i < MAX_PROFILE_CHOICES; i++) {
        if (!pref->isKey(profile_keys[i * 2])) break;
        // key is valid, profile exists
        ++num_existing;
    }
    if (num_existing == MAX_PROFILE_CHOICES) {
        return false;
    }
    profile_idx = num_existing;
    if (!this->name_screen()) {
        return false;
    } else {
        // properly made profile, save data
        // Serial.println(profile_name);
        // Serial.println(profile_data);
        pref->end();
        pref->begin(PREF_NAMESPACE, false);
        pref->putString(profile_keys[profile_idx * 2], profile_name);   // save the profile name
        pref->putString(profile_keys[(profile_idx * 2) + 1], profile_data);   // save the profile data
    }
    return true;
}

uint8_t ProfileEditor::edit_profile(int profile_idx) {
    this->profile_idx = profile_idx;
    // get data from preferences
    pref->getString(profile_keys[profile_idx * 2], profile_name, MAX_PROFILE_NAME_CHARS + 1);
    pref->getString(profile_keys[(profile_idx * 2) + 1], profile_data, MAX_PROFILE_DATA_CHARS + 1);
    
    // setup ec11 choices: Start, Edit, Delete, Back
    ec11->set_rotary_options(NUM_PROFILE_OPTIONS);
    ec11->set_encoder_value(0);

    bool chose_option = false;
    while (!chose_option) {
        int choice = ec11->get_encoder_value();

        display->clear_display_buffer();
        display->clear_text();
        display->println(profile_name);
        display->println("%s Start", choice == 0 ? "[*]" : "[ ]");
        display->println("%s Edit", choice == 1 ? "[*]" : "[ ]");
        display->println("%s Delete", choice == 2 ? "[*]" : "[ ]");
        display->println("%s Back", choice == 3 ? "[*]" : "[ ]");
        display->display_text();

        // select and change states depending on selection
        if (ec11->poll_sw_event() == SW_PRESSED) {
            if (choice == 0) {
                return PROFILE_START;
            } else if (choice == 1) {
                // TODO: might need to set profile_data to zeros here
                
                // setup profile_state, then switch to name and profile edit screens
                for (int profile_ptr = 0; profile_ptr < MAX_PROFILE_DATA_CHARS; profile_ptr += 4) {
                    int time = atoi(profile_data + profile_ptr);
                    profile_ptr += 4;
                    int temp = atoi(profile_data + profile_ptr);
                    profile_state[time / TIME_INTERVALS] = temp / TEMP_INTERVALS;
                }
                if (this->name_screen()) {
                    // properly edited profile, save data
                    pref->end();
                    pref->begin(PREF_NAMESPACE, false);
                    pref->putString(profile_keys[profile_idx * 2], profile_name);   // save the profile name
                    pref->putString(profile_keys[(profile_idx * 2) + 1], profile_data);   // save the profile data
                    return PROFILE_CHANGED;
                } else {
                    return PROFILE_NOTHING;
                }
            } else if (choice == 2) {
                // TODO: delete profile
                pref->end();
                pref->begin(PREF_NAMESPACE, false);
                // shift profiles
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
                return PROFILE_CHANGED;
            } else {
                return PROFILE_NOTHING;
            }
        }
    }
    return PROFILE_NOTHING;
}

bool ProfileEditor::name_screen() {
    // setup ec11 choices
    ec11->set_rotary_options(MAX_PROFILE_NAME_CHARS + 2); // allow rotating between all chars and the "done" and "back" choices
    ec11->set_encoder_value(0);

    int state = CURSOR_STATE;

    bool done_name = false;
    int cursor_position = ec11->get_encoder_value();
    int char_choice = 0;

    // while loop to update ui
    while (!done_name) {
        // current option of knob
        if (state == CURSOR_STATE) {
            cursor_position = ec11->get_encoder_value();
        } else if (state == ALPHA_NUM_STATE) {
            char_choice = ec11->get_encoder_value();
            profile_name[cursor_position] = alpha_num[char_choice];
        }

        // display text
        display->clear_display_buffer();
        display->clear_text();
        display->println("Name:");
        display->println(profile_name);
        for (int i = 0; i < MAX_PROFILE_NAME_CHARS; i++) {
            if (i == cursor_position) {
                display->print("^");
            } else {
                display->print(" ");
            }
        }
        display->newline();
        display->println("%s Done", cursor_position == MAX_PROFILE_NAME_CHARS ? "[*]" : "[ ]");
        display->println("%s Back", cursor_position == (MAX_PROFILE_NAME_CHARS + 1) ? "[*]" : "[ ]");
        display->display_text();

        // select and change states depending on selection
        if (ec11->poll_sw_event() == SW_PRESSED) {
            if (cursor_position < MAX_PROFILE_NAME_CHARS) {
                if (state == CURSOR_STATE) {
                    // transition to edit the char state
                    state = ALPHA_NUM_STATE;
                    // edit the character at the knob_val index
                    ec11->set_rotary_options(NUM_VALID_CHARS);
                    char current_char = profile_name[cursor_position];
                    int start_encoder_value = 0;
                    for (int i = 0; i < NUM_VALID_CHARS; i++) {
                        if (alpha_num[i] == current_char) {
                            start_encoder_value = i;
                            break;
                        }
                    }
                    ec11->set_encoder_value(start_encoder_value);
                } else if (state == ALPHA_NUM_STATE) {
                    // transition to cursor state
                    state = CURSOR_STATE;
                    // done setting the current character, revert ec11 back to cursor mode
                    ec11->set_rotary_options(MAX_PROFILE_NAME_CHARS + 2);
                    ec11->set_encoder_value(cursor_position);
                }
            } else if (cursor_position == MAX_PROFILE_NAME_CHARS) {
                // "done" selection
                // move onto profile screen
                if (!profile_screen()) {
                    // chose to come back to this screen
                    // revert ec11 back to cursor mode
                    ec11->set_rotary_options(MAX_PROFILE_NAME_CHARS + 2);
                    ec11->set_encoder_value(cursor_position);
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
    ec11->set_rotary_options(time_state_options);
    ec11->set_encoder_value(0);
    display->set_graph_limits(0, MAX_TIME - TIME_INTERVALS, 0, MAX_TEMP - TEMP_INTERVALS);

    int state = TIME_STATE;

    bool done_profile = false;
    int time_position = ec11->get_encoder_value();
    int time_value = time_position * TIME_INTERVALS;
    int temp_position = 0;
    int temp_value = temp_position * TEMP_INTERVALS;

    // while loop to update ui
    while (!done_profile) {
        // clear display
        display->clear_display_buffer();
        display->clear_text();
        display->set_axes_labels(time_value, temp_value);   // write values to axes

        if (state == TIME_STATE) {
            // current knob value
            time_position = ec11->get_encoder_value();
            time_value = min(time_position * TIME_INTERVALS, MAX_TIME - TIME_INTERVALS);
            temp_value = profile_state[time_position] * TEMP_INTERVALS; // as the time cursor moves, update the temperature accordingly
            // draw a vertical bar indicating cursor position
            if (time_position < time_value_options) display->draw_graph_tick(time_value, true);
        } else if (state == TEMP_STATE) {
            // current knob value
            temp_position = ec11->get_encoder_value();
            temp_value = temp_position * TEMP_INTERVALS;
            // draw an 'x' corresponding to the current temperature being set
            display->draw_graph_point(time_value, temp_value);
        }
        // display time and temp unless in the "done" or "back" options
        if (time_position < time_value_options) {
            display->set_axes_labels(time_value, temp_value);
        } else {
            display->set_axes_labels("   ", "   ");
        }
        // for every non-zero point in the profile, draw a point
        int profile_nodes = 0;
        for (int i = 0; i < time_value_options; i++) {
            if (profile_state[i] != 0) {
                ++profile_nodes;
                display->draw_graph_point(i * TIME_INTERVALS, profile_state[i] * ((int)TEMP_INTERVALS));
            }
        }
        // display 'done' and 'back' options
        display->print("%s Done   ", time_position == time_value_options ? "[*]" : "[ ]");
        display->println("%s Back", time_position == (time_value_options + 1) ? "[*]" : "[ ]");

        display->display_text(0, SCREEN_HEIGHT - (CHAR_PX_HEIGHT + 2) + 2, false); // Display at the bottom of the screen
        display->display_graph();

        // select and change states depending on selection
        if (ec11->poll_sw_event() == SW_PRESSED) {
            if (time_position < time_value_options) {
                if (state == TIME_STATE) {
                    if (temp_value != 0 || profile_nodes < MAX_PROFILE_DATA_POINTS) {
                        // transition to temperature edit state
                        ec11->set_rotary_options(temp_value_options);
                        ec11->set_encoder_value(temp_value / TEMP_INTERVALS);
                        state = TEMP_STATE;
                    }
                } else if (state == TEMP_STATE) {
                    // set the profile state
                    profile_state[time_position] = temp_position;
                    // transition back to time edit state
                    ec11->set_rotary_options(time_state_options);
                    ec11->set_encoder_value(time_value / TIME_INTERVALS);
                    state = TIME_STATE;
                }
            } else if (time_position == time_value_options) {
                // done selection
                // TODO: loop through profile_state and any non-zero values are made into nodes for the profile
                if (profile_idx < 0) return false; // error

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
                // back selection
                return false;
            }
        }
    }
    return true;
}