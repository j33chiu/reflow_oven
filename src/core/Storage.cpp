#include "core/Storage.h"

// static initialization
Preferences Storage::pref;
bool Storage::is_open = false;
uint8_t Storage::read_write_state = 0;

const char Storage::profile_name_keys[MAX_PROFILE_CHOICES][3] = { // 'n' is the profile name
    "n1", "n2", "n3", "n4", "n5"
};
const char Storage::profile_data_keys[MAX_PROFILE_CHOICES][3] = { // 'n' is the profile name
    "p1", "p2", "p3", "p4", "p5"
};

char Storage::profile_name_cache[MAX_PROFILE_CHOICES][MAX_PROFILE_NAME_CHARS + 1] = {'\0'};
char Storage::profile_data_cache[MAX_PROFILE_CHOICES][MAX_PROFILE_DATA_CHARS + 1] = {'\0'};
bool Storage::profile_name_cache_exists[MAX_PROFILE_CHOICES] = {false};
bool Storage::profile_data_cache_exists[MAX_PROFILE_CHOICES] = {false};

Storage& Storage::get_instance() {
    static Storage instance;
    return instance;
}

Storage::Storage() {

}

bool Storage::setup() {
    nvs_flash_init();
    return true;
}

bool Storage::set_read_only() {
    if (read_write_state == READ_PERM) return true;
    if (pref.begin(PREF_NAMESPACE, true)) {
        is_open = true;
        read_write_state = READ_PERM;
        return true;
    }
    return false;
}

bool Storage::set_read_write() {
    if (read_write_state == READ_WRITE_PERM) return true;
    else if (read_write_state == READ_PERM) {
        // read only currently, stop and restart
        pref.end();
        is_open = false;
        read_write_state = 0;
    }
    if (pref.begin(PREF_NAMESPACE, false)) {
        is_open = true;
        read_write_state = READ_WRITE_PERM;
        return true;
    }
    return false;
}

void Storage::close() {
    if (is_open) pref.end();
    is_open = false;
    read_write_state = 0;
}

uint8_t Storage::read_all() {
    for (int profile_idx = 0; profile_idx < MAX_PROFILE_CHOICES; ++profile_idx) {
        if (!read_profile_name(profile_idx) || !read_profile_data(profile_idx)) {
            return profile_idx;
        }
    }
    return MAX_PROFILE_CHOICES;
}

bool Storage::read_profile(const int profile_idx, char* name_buffer, char* data_buffer) {
    bool result = read_profile_name(profile_idx, name_buffer);
    result = result && read_profile_data(profile_idx, data_buffer);
    return result;
}

bool Storage::read_profile_name(const int profile_idx, char* name_buffer) {
    // if profile index is invalid, return false
    if (profile_idx >= MAX_PROFILE_CHOICES) return false;

    // if cache doesnt exist for the value, read it
    if (!profile_name_cache_exists[profile_idx]) {
        // ensure preferences is open
        if (!is_open && !set_read_only()) {
            return false;
        }
        // ensure the key for the data exists
        if (!pref.isKey(profile_name_keys[profile_idx])) {
            return false;
        }
        // read from preferences
        memset(profile_name_cache[profile_idx], 0, sizeof(profile_name_cache[profile_idx]));
        int len = pref.getString(profile_name_keys[profile_idx], profile_name_cache[profile_idx], MAX_PROFILE_NAME_CHARS + 1);
        // if len == 0, an error occured when trying to read
        if (len == 0) return false; // string write include the null terminator, so a successful write would still be non-zero
        // value is successfully read and cached
        profile_name_cache_exists[profile_idx] = true;
    }

    // the value should be in cache now and it can be returned
    if (name_buffer) snprintf(name_buffer, MAX_PROFILE_NAME_CHARS + 1, profile_name_cache[profile_idx]);
    return true;
}

bool Storage::read_profile_data(const int profile_idx, char* data_buffer) {
    // if profile index is invalid, return false
    if (profile_idx >= MAX_PROFILE_CHOICES) return false;

    // if cache doesnt exist for the value, read it
    if (!profile_data_cache_exists[profile_idx]) {
        // ensure preferences is open
        if (!is_open && !set_read_only()) {
            return false;
        }
        // ensure the key for the data exists
        if (!pref.isKey(profile_data_keys[profile_idx])) {
            return false;
        }
        // read from preferences
        memset(profile_data_cache[profile_idx], 0, sizeof(profile_data_cache[profile_idx]));
        int len = pref.getString(profile_data_keys[profile_idx], profile_data_cache[profile_idx], MAX_PROFILE_DATA_CHARS + 1);
        // if len == 0, an error occured when trying to read
        if (len == 0) return false; // string write include the null terminator, so a successful write would still be non-zero
        // value is successfully read and cached
        profile_data_cache_exists[profile_idx] = true;
    }

    // the value should be in cache now and it can be returned
    if (data_buffer) snprintf(data_buffer, MAX_PROFILE_DATA_CHARS + 1, profile_data_cache[profile_idx]);
    return true;
}

bool Storage::write_profile(const int profile_idx, const char* name_buffer, const char* data_buffer) {
    // if profile index is invalid, return false
    if (profile_idx >= MAX_PROFILE_CHOICES) return false;

    // if name or data are invalid, return false
    if (name_buffer == NULL || data_buffer == NULL) return false;

    // write to cache first
    memset(profile_name_cache[profile_idx], 0, sizeof(profile_name_cache[profile_idx]));
    snprintf(profile_name_cache[profile_idx], MAX_PROFILE_NAME_CHARS + 1, name_buffer);
    profile_name_cache_exists[profile_idx] = true;
    memset(profile_data_cache[profile_idx], 0, sizeof(profile_data_cache[profile_idx]));
    snprintf(profile_data_cache[profile_idx], MAX_PROFILE_DATA_CHARS + 1, data_buffer);
    profile_data_cache_exists[profile_idx] = true;

    // try to overwrite preferences
    // ensure preferences is open and can write
    if ((read_write_state & WRITE_PERM) || set_read_write()) {
        pref.putString(profile_name_keys[profile_idx], name_buffer);
        pref.putString(profile_data_keys[profile_idx], data_buffer);
        return true;
    } else {
        return false;
    }
}

bool Storage::delete_profile(const int profile_idx) {
    // if profile index is invalid, return false
    if (profile_idx >= MAX_PROFILE_CHOICES) return false;

    // shift profiles
    // eg: deleting profile in slot 2 should shift all subsequent slots (3->2, 4->3, and 5->4 etc)
    int i = profile_idx;
    for (i = profile_idx + 1; i < MAX_PROFILE_CHOICES; i++) {
        if (pref.isKey(profile_name_keys[i]) && pref.isKey(profile_data_keys[i])) {
            pref.putString(profile_name_keys[i - 1], profile_name_cache[i]);
            memset(profile_name_cache[i - 1], 0, sizeof(profile_name_cache[i - 1]));
            snprintf(profile_name_cache[i - 1], MAX_PROFILE_NAME_CHARS + 1, profile_name_cache[i]);

            pref.putString(profile_data_keys[i - 1], profile_data_cache[i]);
            memset(profile_data_cache[i - 1], 0, sizeof(profile_data_cache[i - 1]));
            snprintf(profile_data_cache[i - 1], MAX_PROFILE_DATA_CHARS + 1, profile_data_cache[i]);
        } else {
            break;
        }
    }
    --i;
    pref.remove(profile_name_keys[i]);
    pref.remove(profile_data_keys[i]);
    memset(profile_name_cache[i], 0, sizeof(profile_name_cache[i]));
    memset(profile_data_cache[i], 0, sizeof(profile_data_cache[i]));
    profile_name_cache_exists[i] = false;
    profile_data_cache_exists[i] = false;
    return true;
}