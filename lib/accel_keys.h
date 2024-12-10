// The list of control keys for quickscope

// We needed these keys to be defined in one place.  It's used in at least
// two C source files.  So, if we change them here it should be fine so
// long as we do not use GDK_KEY_? in any other C source files.


// If we need to we may add a modifier key like Ctrl or Shift
// for these key presses to take action.
//
#define ControlMod_keys             0


#define ShowHideButtonbar_key       GDK_KEY_b
#define ShowHideMenubar_key         GDK_KEY_m
#define CloseWindow_key             GDK_KEY_c
#define NewTab_key                  GDK_KEY_t
#define NewWindow_key               GDK_KEY_n
#define Quit_key                    GDK_KEY_q

