#ifndef STORAGE_H
#define STORAGE_H

#define PREF_NAMESPACE "pref_rf_oven"

#define READ_PERM           0b10
#define WRITE_PERM          0b01
#define READ_WRITE_PERM     0b11

// profile defines
#define MAX_PROFILE_CHOICES     5                             // max profiles that can be saved
#define MAX_PROFILE_NAME_CHARS  16                            // max number of chars that can be saved for a profile name
#define MAX_PROFILE_DATA_POINTS 32                            // max number of nodes
#define MAX_PROFILE_DATA_CHARS  (MAX_PROFILE_DATA_POINTS * 8) // max number of chars that can be saved for a profile's data (timing and temp data), each node takes 8 chars to store, 

// access profile info from preferences
const char profile_keys[MAX_PROFILE_CHOICES * 2][3] = { // 'n' is the profile name, 'p' is the actual profile data
  "n1", "p1",
  "n2", "p2",
  "n3", "p3",
  "n4", "p4",
  "n5", "p5",
};

#include <nvs_flash.h>
#include <Preferences.h>

// singleton class for accessing flash storage on the esp32
class Storage {
public:
    // make assignment and copy inaccesible
    Storage(Storage const&) = delete;
    Storage(Storage &&) = delete;
    Storage& operator=(Storage const&) = delete;
    Storage& operator=(Storage &&) = delete;

    static Storage& get_instance();
    bool setup();

    // initialization and setup of Preferences
    static bool set_read_only();
    static bool set_read_write();
    static void close();

    // read data
    static uint8_t read_all();  // returns number of existing profiles
    static bool read_profile(const int profile_idx, char* name_buffer = nullptr, char* data_buffer = nullptr);
    static bool read_profile_name(const int profile_idx, char* name_buffer = nullptr);
    static bool read_profile_data(const int profile_idx, char* data_buffer = nullptr);

    // write data
    static bool write_profile(const int profile_idx, const char* name_buffer, const char* data_buffer);
    static bool delete_profile(const int profile_idx);


private:
    Storage();
    static Preferences pref;
    static bool is_open;
    static uint8_t read_write_state;

    static const char profile_name_keys[MAX_PROFILE_CHOICES][3];
    static const char profile_data_keys[MAX_PROFILE_CHOICES][3];

    static char profile_name_cache[MAX_PROFILE_CHOICES][MAX_PROFILE_NAME_CHARS + 1];
    static char profile_data_cache[MAX_PROFILE_CHOICES][MAX_PROFILE_DATA_CHARS + 1];
    static bool profile_name_cache_exists[MAX_PROFILE_CHOICES];
    static bool profile_data_cache_exists[MAX_PROFILE_CHOICES];
};

#endif