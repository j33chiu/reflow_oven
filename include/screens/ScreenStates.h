// Application state machine
#define STATE_IDLE              0 
#define STATE_PROFILES_LIST     1       // list all available profiles, with +new profile option up to MAX_PROFILE_CHOICES
#define STATE_PROFILE_OPTIONS   2       // edit, delete or run options for the currently selected profile
#define STATE_REFLOWING         3       // entire reflow profile, skips individual zones from typical reflow profiles, follows precisely set profile
#define STATE_MANUAL            10      // following manually set heating temperature target, eg via knob (meant as a debug mode)
#define STATE_DEBUG             20      // debug mode
#define STATE_CURSOR            0x100
#define STATE_DATA_EDIT         0x200

// universal options
#define OPTION_BACK     0x100
#define OPTION_DONE     0x200
#define OPTION_NONE     0x300

// idle screen
#define IDLE_OPTIONS    3   // profile list, manual mode, debug mode

#define IDLE_OPTION_PROFILES    0x10
#define IDLE_OPTION_MANUAL      0x20
#define IDLE_OPTION_DEBUG       0x30

// profile list screen
#define PROFILES_CHOICE     0x10
#define PROFILES_NEW        0x20

// profile screen
#define PROFILE_OPTIONS     4   // Start, Edit, Delete, Back

#define PROFILE_OPTION_START    0x10
#define PROFILE_OPTION_EDIT     0x20
#define PROFILE_OPTION_DELETE   0x30

// profile editor screen
#define STATE_NAME_EDITOR       0x10
#define STATE_CURVE_EDITOR      0x20

#define PROFILE_EDITOR_ERROR    0x10
#define PROFILE_EDITOR_DONE     0x20
