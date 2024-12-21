// The list of control keys for quickscope

// We needed these keys to be defined in one place.  It's used in at least
// two C source files.  So, if we change them here it should be fine so
// long as we do not use GDK_KEY_? in any C source files, but this one.


// If we need to, we may add a modifier key like Ctrl or Shift
// for these key presses to take action.
//

#define MOD_SHIFT                   01   

#define PopZoom_key                 GDK_KEY_z
#define ShowHideMenubar_key         GDK_KEY_m
#define ShowHideTabbar_key          GDK_KEY_t
#define ShowHideButtonbar_key       GDK_KEY_b
#define ShowHideStatusbar_key       GDK_KEY_s
#define ShowHideControlbar_key      GDK_KEY_c
#define NewTab_key                  GDK_KEY_g
#define CloseTab_key                GDK_KEY_g // with MOD_SHIFT
#define NewWindow_key               GDK_KEY_w
#define CloseWindow_key             GDK_KEY_w // with MOD_SHIFT
#define Quit_key                    GDK_KEY_q

