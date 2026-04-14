// SDL3-to-SDL2 compatibility header for DuckStation.
// Provides SDL3-named constants, types, and functions using SDL2 as the backend.
// Only covers the subset of SDL3 API that DuckStation actually uses.
// NOTE: Does NOT include SDL_syswm.h to avoid X11 macro pollution.

#pragma once

#include <SDL.h>

#include <cstdlib>
#include <cstring>

// ============================================================================
// Init flags
// ============================================================================
#define SDL_INIT_GAMEPAD SDL_INIT_GAMECONTROLLER

// ============================================================================
// Type aliases
// ============================================================================
typedef SDL_GameController SDL_Gamepad;
typedef SDL_GameControllerAxis SDL_GamepadAxis;
typedef SDL_GameControllerButton SDL_GamepadButton;
typedef SDL_GameControllerType SDL_GamepadType;

// ============================================================================
// Event type mapping
// ============================================================================
#define SDL_EVENT_QUIT SDL_QUIT
#define SDL_EVENT_KEY_DOWN SDL_KEYDOWN
#define SDL_EVENT_KEY_UP SDL_KEYUP
#define SDL_EVENT_MOUSE_MOTION SDL_MOUSEMOTION
#define SDL_EVENT_MOUSE_BUTTON_DOWN SDL_MOUSEBUTTONDOWN
#define SDL_EVENT_MOUSE_BUTTON_UP SDL_MOUSEBUTTONUP
#define SDL_EVENT_MOUSE_WHEEL SDL_MOUSEWHEEL
#define SDL_EVENT_TEXT_INPUT SDL_TEXTINPUT
#define SDL_EVENT_WINDOW_RESIZED SDL_WINDOWEVENT

// Gamepad (controller) events
#define SDL_EVENT_GAMEPAD_ADDED SDL_CONTROLLERDEVICEADDED
#define SDL_EVENT_GAMEPAD_REMOVED SDL_CONTROLLERDEVICEREMOVED
#define SDL_EVENT_GAMEPAD_AXIS_MOTION SDL_CONTROLLERAXISMOTION
#define SDL_EVENT_GAMEPAD_BUTTON_DOWN SDL_CONTROLLERBUTTONDOWN
#define SDL_EVENT_GAMEPAD_BUTTON_UP SDL_CONTROLLERBUTTONUP
#define SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN SDL_CONTROLLERTOUCHPADDOWN
#define SDL_EVENT_GAMEPAD_TOUCHPAD_UP SDL_CONTROLLERTOUCHPADUP
#define SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION SDL_CONTROLLERTOUCHPADMOTION
#define SDL_EVENT_GAMEPAD_SENSOR_UPDATE SDL_CONTROLLERSENSORUPDATE

// Joystick events
#define SDL_EVENT_JOYSTICK_ADDED SDL_JOYDEVICEADDED
#define SDL_EVENT_JOYSTICK_REMOVED SDL_JOYDEVICEREMOVED
#define SDL_EVENT_JOYSTICK_AXIS_MOTION SDL_JOYAXISMOTION
#define SDL_EVENT_JOYSTICK_BUTTON_DOWN SDL_JOYBUTTONDOWN
#define SDL_EVENT_JOYSTICK_BUTTON_UP SDL_JOYBUTTONUP
#define SDL_EVENT_JOYSTICK_HAT_MOTION SDL_JOYHATMOTION

// ============================================================================
// Event struct member aliases (SDL_Event union members)
// SDL3: event.gaxis/gbutton/gdevice/gtouchpad/gsensor
// SDL2: event.caxis/cbutton/cdevice/ctouchpad/csensor
// ============================================================================
#define gaxis caxis
#define gbutton cbutton
#define gdevice cdevice
#define gtouchpad ctouchpad
#define gsensor csensor

// ============================================================================
// Event struct type aliases
// ============================================================================
typedef SDL_ControllerAxisEvent SDL_GamepadAxisEvent;
typedef SDL_ControllerButtonEvent SDL_GamepadButtonEvent;
typedef SDL_ControllerDeviceEvent SDL_GamepadDeviceEvent;
typedef SDL_ControllerTouchpadEvent SDL_GamepadTouchpadEvent;
typedef SDL_ControllerSensorEvent SDL_GamepadSensorEvent;

// ============================================================================
// Gamepad axis constants
// ============================================================================
#define SDL_GAMEPAD_AXIS_LEFTX SDL_CONTROLLER_AXIS_LEFTX
#define SDL_GAMEPAD_AXIS_LEFTY SDL_CONTROLLER_AXIS_LEFTY
#define SDL_GAMEPAD_AXIS_RIGHTX SDL_CONTROLLER_AXIS_RIGHTX
#define SDL_GAMEPAD_AXIS_RIGHTY SDL_CONTROLLER_AXIS_RIGHTY
#define SDL_GAMEPAD_AXIS_LEFT_TRIGGER SDL_CONTROLLER_AXIS_TRIGGERLEFT
#define SDL_GAMEPAD_AXIS_RIGHT_TRIGGER SDL_CONTROLLER_AXIS_TRIGGERRIGHT
#define SDL_GAMEPAD_AXIS_COUNT SDL_CONTROLLER_AXIS_MAX

// ============================================================================
// Gamepad button constants
// ============================================================================
#define SDL_GAMEPAD_BUTTON_SOUTH SDL_CONTROLLER_BUTTON_A
#define SDL_GAMEPAD_BUTTON_EAST SDL_CONTROLLER_BUTTON_B
#define SDL_GAMEPAD_BUTTON_WEST SDL_CONTROLLER_BUTTON_X
#define SDL_GAMEPAD_BUTTON_NORTH SDL_CONTROLLER_BUTTON_Y
#define SDL_GAMEPAD_BUTTON_BACK SDL_CONTROLLER_BUTTON_BACK
#define SDL_GAMEPAD_BUTTON_GUIDE SDL_CONTROLLER_BUTTON_GUIDE
#define SDL_GAMEPAD_BUTTON_START SDL_CONTROLLER_BUTTON_START
#define SDL_GAMEPAD_BUTTON_LEFT_STICK SDL_CONTROLLER_BUTTON_LEFTSTICK
#define SDL_GAMEPAD_BUTTON_RIGHT_STICK SDL_CONTROLLER_BUTTON_RIGHTSTICK
#define SDL_GAMEPAD_BUTTON_LEFT_SHOULDER SDL_CONTROLLER_BUTTON_LEFTSHOULDER
#define SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
#define SDL_GAMEPAD_BUTTON_DPAD_UP SDL_CONTROLLER_BUTTON_DPAD_UP
#define SDL_GAMEPAD_BUTTON_DPAD_DOWN SDL_CONTROLLER_BUTTON_DPAD_DOWN
#define SDL_GAMEPAD_BUTTON_DPAD_LEFT SDL_CONTROLLER_BUTTON_DPAD_LEFT
#define SDL_GAMEPAD_BUTTON_DPAD_RIGHT SDL_CONTROLLER_BUTTON_DPAD_RIGHT
#define SDL_GAMEPAD_BUTTON_MISC1 SDL_CONTROLLER_BUTTON_MISC1
#define SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1 SDL_CONTROLLER_BUTTON_PADDLE1
#define SDL_GAMEPAD_BUTTON_LEFT_PADDLE1 SDL_CONTROLLER_BUTTON_PADDLE2
#define SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2 SDL_CONTROLLER_BUTTON_PADDLE3
#define SDL_GAMEPAD_BUTTON_LEFT_PADDLE2 SDL_CONTROLLER_BUTTON_PADDLE4
#define SDL_GAMEPAD_BUTTON_TOUCHPAD SDL_CONTROLLER_BUTTON_TOUCHPAD
// SDL3 buttons beyond SDL2's max
#define SDL_GAMEPAD_BUTTON_MISC2 (SDL_CONTROLLER_BUTTON_MAX + 0)
#define SDL_GAMEPAD_BUTTON_MISC3 (SDL_CONTROLLER_BUTTON_MAX + 1)
#define SDL_GAMEPAD_BUTTON_MISC4 (SDL_CONTROLLER_BUTTON_MAX + 2)
#define SDL_GAMEPAD_BUTTON_MISC5 (SDL_CONTROLLER_BUTTON_MAX + 3)
#define SDL_GAMEPAD_BUTTON_MISC6 (SDL_CONTROLLER_BUTTON_MAX + 4)
#define SDL_GAMEPAD_BUTTON_COUNT (SDL_CONTROLLER_BUTTON_MAX + 5)

// ============================================================================
// Gamepad type constants
// ============================================================================
#define SDL_GAMEPAD_TYPE_UNKNOWN SDL_CONTROLLER_TYPE_UNKNOWN
#define SDL_GAMEPAD_TYPE_STANDARD SDL_CONTROLLER_TYPE_UNKNOWN
#define SDL_GAMEPAD_TYPE_XBOX360 SDL_CONTROLLER_TYPE_XBOX360
#define SDL_GAMEPAD_TYPE_XBOXONE SDL_CONTROLLER_TYPE_XBOXONE
#define SDL_GAMEPAD_TYPE_PS3 SDL_CONTROLLER_TYPE_PS3
#define SDL_GAMEPAD_TYPE_PS4 SDL_CONTROLLER_TYPE_PS4
#define SDL_GAMEPAD_TYPE_PS5 SDL_CONTROLLER_TYPE_PS5
#define SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO
#define SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT
#define SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT
#define SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR
#define SDL_GAMEPAD_TYPE_GAMECUBE SDL_CONTROLLER_TYPE_UNKNOWN
// Must match the number of entries in DuckStation's s_sdl_gamepad_type_names array (12)
#define SDL_GAMEPAD_TYPE_COUNT 12

// ============================================================================
// Gamepad binding
// ============================================================================
#define SDL_GAMEPAD_BINDTYPE_BUTTON SDL_CONTROLLER_BINDTYPE_BUTTON
#define SDL_GAMEPAD_BINDTYPE_AXIS SDL_CONTROLLER_BINDTYPE_AXIS
#define SDL_GAMEPAD_BINDTYPE_HAT SDL_CONTROLLER_BINDTYPE_HAT

// SDL3's GamepadBinding has different member names than SDL2's GameControllerButtonBind.
// SDL3: .input_type, .input.*, .output_type, .output.*
// SDL2: .bindType, .value.* (no output side — different design)
// We typedef and handle the difference where used.
typedef SDL_GameControllerButtonBind SDL_GamepadBinding;

// ============================================================================
// Gamepad button label enum (SDL3 only - stub for SDL2)
// ============================================================================
enum SDL_GamepadButtonLabel
{
  SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN = 0,
  SDL_GAMEPAD_BUTTON_LABEL_A,
  SDL_GAMEPAD_BUTTON_LABEL_B,
  SDL_GAMEPAD_BUTTON_LABEL_X,
  SDL_GAMEPAD_BUTTON_LABEL_Y,
  SDL_GAMEPAD_BUTTON_LABEL_CROSS,
  SDL_GAMEPAD_BUTTON_LABEL_CIRCLE,
  SDL_GAMEPAD_BUTTON_LABEL_SQUARE,
  SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE,
};

// ============================================================================
// Sensor constants
// SDL2 has SDL_SensorType as an enum. SDL_SENSOR_ACCEL exists in SDL2 2.0.14+.
// ============================================================================

// ============================================================================
// Window flags
// ============================================================================
#define SDL_WINDOW_HIGH_PIXEL_DENSITY SDL_WINDOW_ALLOW_HIGHDPI

// ============================================================================
// Hint name mapping — only define hints SDL2 doesn't already have.
// Don't redefine hints that SDL2 already provides (self-referential macros break).
// ============================================================================
#ifndef SDL_HINT_JOYSTICK_DIRECTINPUT
#define SDL_HINT_JOYSTICK_DIRECTINPUT "SDL_JOYSTICK_DIRECTINPUT"
#endif
#ifndef SDL_HINT_JOYSTICK_ENHANCED_REPORTS
#define SDL_HINT_JOYSTICK_ENHANCED_REPORTS "SDL_JOYSTICK_HIDAPI_PS5_RUMBLE"
#endif
#ifndef SDL_HINT_JOYSTICK_GAMEINPUT
#define SDL_HINT_JOYSTICK_GAMEINPUT "SDL_JOYSTICK_GAMEINPUT"
#endif
#ifndef SDL_HINT_JOYSTICK_HIDAPI_XBOX
#define SDL_HINT_JOYSTICK_HIDAPI_XBOX "SDL_JOYSTICK_HIDAPI_XBOX"
#endif
#ifndef SDL_HINT_JOYSTICK_LINUX_DIGITAL_HATS
#define SDL_HINT_JOYSTICK_LINUX_DIGITAL_HATS "SDL_JOYSTICK_LINUX_DIGITAL_HATS"
#endif
#ifndef SDL_HINT_JOYSTICK_HIDAPI_PS3
#define SDL_HINT_JOYSTICK_HIDAPI_PS3 "SDL_JOYSTICK_HIDAPI_PS3"
#endif
#ifndef SDL_HINT_JOYSTICK_HIDAPI_PS5_PLAYER_LED
#define SDL_HINT_JOYSTICK_HIDAPI_PS5_PLAYER_LED "SDL_JOYSTICK_HIDAPI_PS5_PLAYER_LED"
#endif
#ifndef SDL_HINT_JOYSTICK_HIDAPI_WII
#define SDL_HINT_JOYSTICK_HIDAPI_WII "SDL_JOYSTICK_HIDAPI_WII"
#endif
#ifndef SDL_HINT_JOYSTICK_RAWINPUT
#define SDL_HINT_JOYSTICK_RAWINPUT "SDL_JOYSTICK_RAWINPUT"
#endif
#ifndef SDL_HINT_JOYSTICK_IOKIT
#define SDL_HINT_JOYSTICK_IOKIT "SDL_JOYSTICK_IOKIT"
#endif
#ifndef SDL_HINT_JOYSTICK_MFI
#define SDL_HINT_JOYSTICK_MFI "SDL_JOYSTICK_MFI"
#endif
#ifndef SDL_HINT_JOYSTICK_WGI
#define SDL_HINT_JOYSTICK_WGI "SDL_JOYSTICK_WGI"
#endif
#ifndef SDL_HINT_XINPUT_ENABLED
#define SDL_HINT_XINPUT_ENABLED "SDL_XINPUT_ENABLED"
#endif

// ============================================================================
// Log priority mapping
// ============================================================================
#define SDL_LOG_PRIORITY_TRACE SDL_LOG_PRIORITY_VERBOSE
#define SDL_LOG_PRIORITY_INVALID SDL_NUM_LOG_PRIORITIES
#define SDL_LOG_PRIORITY_COUNT SDL_NUM_LOG_PRIORITIES
#define SDL_SetLogOutputFunction SDL_LogSetOutputFunction
#define SDL_SetLogPriorities SDL_LogSetAllPriority

// ============================================================================
// Haptic constants and function renames
// ============================================================================
#ifndef SDL_MAX_RUMBLE_DURATION_MS
#define SDL_MAX_RUMBLE_DURATION_MS 0xFFFFFFFF
#endif
#define SDL_CreateHapticEffect SDL_HapticNewEffect
#define SDL_UpdateHapticEffect SDL_HapticUpdateEffect
#define SDL_RunHapticEffect SDL_HapticRunEffect
#define SDL_StopHapticEffect SDL_HapticStopEffect
#define SDL_DestroyHapticEffect SDL_HapticDestroyEffect
#define SDL_StopHapticRumble SDL_HapticRumbleStop
#define SDL_GetHapticFeatures SDL_HapticQuery

// ============================================================================
// Misc constants
// ============================================================================
#ifndef SDL_STANDARD_GRAVITY
#define SDL_STANDARD_GRAVITY 9.80665f
#endif
// NOTE: SDL_DEFAULT_XBOX_HIDAPI and SDL_hidapi_ps5 are local variables in the
// input source, not SDL defines. Do not define them here.

// ============================================================================
// Properties API (SDL3 only — stub for SDL2)
// ============================================================================
typedef Uint32 SDL_PropertiesID;

static inline SDL_PropertiesID SDL_GetGamepadProperties(SDL_GameController*) { return 0; }
static inline SDL_PropertiesID SDL_GetJoystickProperties(SDL_Joystick*) { return 0; }
static inline SDL_PropertiesID SDL_GetWindowProperties(SDL_Window*) { return 0; }
static inline bool SDL_GetBooleanProperty(SDL_PropertiesID, const char*, bool def) { return def; }
static inline void* SDL_GetPointerProperty(SDL_PropertiesID, const char*, void* def) { return def; }
static inline Sint64 SDL_GetNumberProperty(SDL_PropertiesID, const char*, Sint64 def) { return def; }

#define SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN "SDL.gamepad.cap.rumble"
#define SDL_PROP_GAMEPAD_CAP_RGB_LED_BOOLEAN "SDL.gamepad.cap.rgb_led"
#define SDL_PROP_GAMEPAD_CAP_MONO_LED_BOOLEAN "SDL.gamepad.cap.mono_led"
#define SDL_PROP_WINDOW_X11_DISPLAY_POINTER "SDL.window.x11.display"
#define SDL_PROP_WINDOW_X11_WINDOW_NUMBER "SDL.window.x11.window"
#define SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER "SDL.window.wayland.display"
#define SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER "SDL.window.wayland.surface"

// ============================================================================
// Functions: bool-returning wrappers for SDL2 int-returning functions
// ============================================================================
namespace SDL3Compat {
static inline bool Init(Uint32 flags) { return ::SDL_Init(flags) == 0; }
static inline bool InitSubSystem(Uint32 flags) { return ::SDL_InitSubSystem(flags) == 0; }
static inline bool InitHapticRumble(SDL_Haptic* h) { return ::SDL_HapticRumbleInit(h) == 0; }
static inline bool PlayHapticRumble(SDL_Haptic* h, float s, Uint32 l) { return ::SDL_HapticRumblePlay(h, s, l) == 0; }
static inline bool _DisableScreenSaver() { ::SDL_DisableScreenSaver(); return true; }
static inline bool _EnableScreenSaver() { ::SDL_EnableScreenSaver(); return true; }
static inline bool SetGamepadSensorEnabled(SDL_GameController* gp, SDL_SensorType t, bool e)
{
  return ::SDL_GameControllerSetSensorEnabled(gp, t, e ? SDL_TRUE : SDL_FALSE) == 0;
}
static inline bool RumbleGamepad(SDL_GameController* gp, Uint16 lo, Uint16 hi, Uint32 dur)
{
  return ::SDL_GameControllerRumble(gp, lo, hi, dur) == 0;
}
static inline bool SetGamepadLED(SDL_GameController* gp, Uint8 r, Uint8 g, Uint8 b)
{
  return ::SDL_GameControllerSetLED(gp, r, g, b) == 0;
}
static inline bool SendGamepadEffect(SDL_GameController* gp, const void* data, int size)
{
  return ::SDL_GameControllerSendEffect(gp, data, size) == 0;
}
} // namespace SDL3Compat

#undef SDL_Init
#define SDL_Init SDL3Compat::Init
#undef SDL_InitSubSystem
#define SDL_InitSubSystem SDL3Compat::InitSubSystem
#define SDL_InitHapticRumble SDL3Compat::InitHapticRumble
#define SDL_PlayHapticRumble SDL3Compat::PlayHapticRumble
#undef SDL_DisableScreenSaver
#define SDL_DisableScreenSaver SDL3Compat::_DisableScreenSaver
#undef SDL_EnableScreenSaver
#define SDL_EnableScreenSaver SDL3Compat::_EnableScreenSaver
#define SDL_SetGamepadSensorEnabled SDL3Compat::SetGamepadSensorEnabled
#define SDL_RumbleGamepad SDL3Compat::RumbleGamepad
#define SDL_SetGamepadLED SDL3Compat::SetGamepadLED
#define SDL_SendGamepadEffect SDL3Compat::SendGamepadEffect

// ============================================================================
// Gamepad functions (renamed from GameController)
// ============================================================================
#define SDL_OpenGamepad SDL_GameControllerOpen
#define SDL_CloseGamepad SDL_GameControllerClose
#define SDL_GetGamepadAxis SDL_GameControllerGetAxis
#define SDL_GetGamepadButton SDL_GameControllerGetButton
#define SDL_GetGamepadJoystick SDL_GameControllerGetJoystick
static inline const char* SDL3_GetGamepadName(SDL_GameController* gp) { return SDL_GameControllerGetName(gp); }
#define SDL_GetGamepadName SDL3_GetGamepadName
#define SDL_GetGamepadPlayerIndex SDL_GameControllerGetPlayerIndex
#define SDL_GetGamepadVendor SDL_GameControllerGetVendor
#define SDL_GetGamepadProduct SDL_GameControllerGetProduct
#define SDL_IsGamepad SDL_IsGameController
#define SDL_GamepadHasSensor SDL_GameControllerHasSensor

static inline SDL_GameControllerType SDL3_GetGamepadType(SDL_GameController* gp)
{
  return SDL_GameControllerGetType(gp);
}
#define SDL_GetGamepadType SDL3_GetGamepadType

// ============================================================================
// Gamepad button label (SDL3 only — stub)
// ============================================================================
static inline SDL_GamepadButtonLabel SDL3_GetGamepadButtonLabelForType(SDL_GameControllerType type,
                                                                       SDL_GameControllerButton button)
{
  const bool is_ps = (type == SDL_CONTROLLER_TYPE_PS3 || type == SDL_CONTROLLER_TYPE_PS4 ||
                      type == SDL_CONTROLLER_TYPE_PS5);
  switch (button)
  {
    case SDL_CONTROLLER_BUTTON_A:
      return is_ps ? SDL_GAMEPAD_BUTTON_LABEL_CROSS : SDL_GAMEPAD_BUTTON_LABEL_A;
    case SDL_CONTROLLER_BUTTON_B:
      return is_ps ? SDL_GAMEPAD_BUTTON_LABEL_CIRCLE : SDL_GAMEPAD_BUTTON_LABEL_B;
    case SDL_CONTROLLER_BUTTON_X:
      return is_ps ? SDL_GAMEPAD_BUTTON_LABEL_SQUARE : SDL_GAMEPAD_BUTTON_LABEL_X;
    case SDL_CONTROLLER_BUTTON_Y:
      return is_ps ? SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE : SDL_GAMEPAD_BUTTON_LABEL_Y;
    default:
      return SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN;
  }
}
#define SDL_GetGamepadButtonLabelForType SDL3_GetGamepadButtonLabelForType

// ============================================================================
// Gamepad bindings (SDL3 changed the API)
// ============================================================================
static inline SDL_GameControllerButtonBind* SDL3_GetGamepadBindings(SDL_GameController* gp, int* count)
{
  int n = 0;
  for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; i++)
  {
    SDL_GameControllerButtonBind b = SDL_GameControllerGetBindForAxis(gp, (SDL_GameControllerAxis)i);
    if (b.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
      n++;
  }
  for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
  {
    SDL_GameControllerButtonBind b = SDL_GameControllerGetBindForButton(gp, (SDL_GameControllerButton)i);
    if (b.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
      n++;
  }

  auto* result = (SDL_GameControllerButtonBind*)SDL_malloc(sizeof(SDL_GameControllerButtonBind) * (n > 0 ? n : 1));
  int idx = 0;
  for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX && idx < n; i++)
  {
    SDL_GameControllerButtonBind b = SDL_GameControllerGetBindForAxis(gp, (SDL_GameControllerAxis)i);
    if (b.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
      result[idx++] = b;
  }
  for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX && idx < n; i++)
  {
    SDL_GameControllerButtonBind b = SDL_GameControllerGetBindForButton(gp, (SDL_GameControllerButton)i);
    if (b.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
      result[idx++] = b;
  }

  *count = n;
  return result;
}
#define SDL_GetGamepadBindings SDL3_GetGamepadBindings

// ============================================================================
// Gamepad mappings (SDL3 changed the API)
// ============================================================================
static inline char** SDL3_GetGamepadMappings(int* count)
{
  *count = SDL_GameControllerNumMappings();
  char** result = (char**)SDL_malloc(sizeof(char*));
  result[0] = nullptr;
  return result;
}
#define SDL_GetGamepadMappings SDL3_GetGamepadMappings

// ============================================================================
// Joystick functions (renames)
// ============================================================================
#define SDL_GetJoystickID SDL_JoystickInstanceID
#define SDL_GetJoystickName SDL_JoystickName
#define SDL_GetJoystickGUID SDL_JoystickGetGUID
#define SDL_GetJoystickPlayerIndex SDL_JoystickGetPlayerIndex
#define SDL_GetJoystickAxis SDL_JoystickGetAxis
#define SDL_GetJoystickButton SDL_JoystickGetButton
#define SDL_GetJoystickHat SDL_JoystickGetHat
#define SDL_GetNumJoystickAxes SDL_JoystickNumAxes
#define SDL_GetNumJoystickButtons SDL_JoystickNumButtons
#define SDL_GetNumJoystickHats SDL_JoystickNumHats
#define SDL_OpenJoystick SDL_JoystickOpen
#define SDL_CloseJoystick SDL_JoystickClose
#define SDL_OpenHapticFromJoystick SDL_HapticOpenFromJoystick
#define SDL_GUIDToString SDL_JoystickGetGUIDString

// ============================================================================
// Window functions
// ============================================================================
static inline SDL_Window* SDL3_CreateWindow(const char* title, int w, int h, Uint32 flags)
{
  return ::SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
}
#undef SDL_CreateWindow
#define SDL_CreateWindow SDL3_CreateWindow

static inline void SDL3_GetWindowSizeInPixels(SDL_Window* win, int* w, int* h)
{
  SDL_GL_GetDrawableSize(win, w, h);
  if (*w == 0 || *h == 0)
    SDL_GetWindowSize(win, w, h);
}
#define SDL_GetWindowSizeInPixels SDL3_GetWindowSizeInPixels

static inline float SDL3_GetWindowDisplayScale(SDL_Window*) { return 1.0f; }
#define SDL_GetWindowDisplayScale SDL3_GetWindowDisplayScale

typedef int SDL_DisplayID;
static inline SDL_DisplayID SDL3_GetDisplayForWindow(SDL_Window* win)
{
  int idx = SDL_GetWindowDisplayIndex(win);
  return (idx >= 0) ? (idx + 1) : 0;
}
#define SDL_GetDisplayForWindow SDL3_GetDisplayForWindow

static inline const SDL_DisplayMode* SDL3_GetCurrentDisplayMode(SDL_DisplayID display_id)
{
  static thread_local SDL_DisplayMode s_mode;
  if (display_id > 0 && ::SDL_GetCurrentDisplayMode(display_id - 1, &s_mode) == 0)
    return &s_mode;
  return nullptr;
}
#undef SDL_GetCurrentDisplayMode
#define SDL_GetCurrentDisplayMode SDL3_GetCurrentDisplayMode

// ============================================================================
// Mouse / cursor / text input
// ============================================================================
static inline bool SDL3_SetWindowRelativeMouseMode(SDL_Window*, bool enabled)
{
  return SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE) == 0;
}
#define SDL_SetWindowRelativeMouseMode SDL3_SetWindowRelativeMouseMode

static inline void SDL3_HideCursor() { ::SDL_ShowCursor(SDL_DISABLE); }
static inline void SDL3_ShowCursorCompat() { ::SDL_ShowCursor(SDL_ENABLE); }
#undef SDL_HideCursor
#define SDL_HideCursor SDL3_HideCursor
#undef SDL_ShowCursor
#define SDL_ShowCursor SDL3_ShowCursorCompat

static inline void SDL3_StartTextInput(SDL_Window*) { ::SDL_StartTextInput(); }
static inline void SDL3_StopTextInput(SDL_Window*) { ::SDL_StopTextInput(); }
#undef SDL_StartTextInput
#define SDL_StartTextInput SDL3_StartTextInput
#undef SDL_StopTextInput
#define SDL_StopTextInput SDL3_StopTextInput
