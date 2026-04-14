// SDL3-to-SDL2 compatibility header for DuckStation.
// Provides SDL3-named constants, types, and functions using SDL2 as the backend.
// Only covers the subset of SDL3 API that DuckStation actually uses.

#pragma once

#include <SDL.h>
#include <SDL_syswm.h>

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

// SDL3 uses Uint32 for JoystickID, SDL2 uses Sint32. Typedef kept as-is since
// SDL_JoystickID exists in both. Signed/unsigned difference is benign in practice.

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

// Window events - SDL3 splits these out, SDL2 groups under SDL_WINDOWEVENT.
// Our frontend handles this specially; the input source doesn't use window events.
#define SDL_EVENT_WINDOW_RESIZED SDL_WINDOWEVENT

// ============================================================================
// Event struct member aliases (SDL_Event union members)
// SDL3: event.gaxis, event.gbutton, event.gdevice, event.gtouchpad, event.gsensor
// SDL2: event.caxis, event.cbutton, event.cdevice, event.ctouchpad, event.csensor
// Joystick members (jaxis, jbutton, jhat, jdevice) are the same in both.
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
typedef SDL_JoyAxisEvent SDL_JoyAxisEvent; // same name
typedef SDL_JoyButtonEvent SDL_JoyButtonEvent; // same name
typedef SDL_JoyHatEvent SDL_JoyHatEvent; // same name

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
// SDL3 added extra buttons not in SDL2. Define them past SDL2's max.
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
// SDL3 added GameCube type; not in SDL2
#define SDL_GAMEPAD_TYPE_GAMECUBE SDL_CONTROLLER_TYPE_UNKNOWN
#define SDL_GAMEPAD_TYPE_COUNT (SDL_CONTROLLER_TYPE_NVIDIA_SHIELD + 2)

typedef SDL_GameControllerType SDL_GamepadType;

// ============================================================================
// Gamepad binding type constants
// ============================================================================
#define SDL_GAMEPAD_BINDTYPE_BUTTON SDL_CONTROLLER_BINDTYPE_BUTTON
#define SDL_GAMEPAD_BINDTYPE_AXIS SDL_CONTROLLER_BINDTYPE_AXIS
#define SDL_GAMEPAD_BINDTYPE_HAT SDL_CONTROLLER_BINDTYPE_HAT

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
// ============================================================================
#ifndef SDL_SENSOR_ACCEL
#define SDL_SENSOR_ACCEL SDL_SENSOR_ACCEL_L
// If SDL2 doesn't have sensor constants, define them
#endif

// ============================================================================
// Window flags
// ============================================================================
#define SDL_WINDOW_HIGH_PIXEL_DENSITY SDL_WINDOW_ALLOW_HIGHDPI

// ============================================================================
// Hint name mapping (SDL3 renamed some hints)
// ============================================================================
#define SDL_HINT_GAMECONTROLLERCONFIG_FILE SDL_HINT_GAMECONTROLLERCONFIG_FILE
#define SDL_HINT_JOYSTICK_DIRECTINPUT "SDL_JOYSTICK_DIRECTINPUT"
#define SDL_HINT_JOYSTICK_ENHANCED_REPORTS "SDL_JOYSTICK_HIDAPI_PS5_RUMBLE"
#define SDL_HINT_JOYSTICK_GAMEINPUT "SDL_JOYSTICK_GAMEINPUT"
#define SDL_HINT_JOYSTICK_HIDAPI_PS3 SDL_HINT_JOYSTICK_HIDAPI_PS3
#define SDL_HINT_JOYSTICK_HIDAPI_PS5_PLAYER_LED SDL_HINT_JOYSTICK_HIDAPI_PS5_PLAYER_LED
#define SDL_HINT_JOYSTICK_HIDAPI_WII SDL_HINT_JOYSTICK_HIDAPI_WII
#define SDL_HINT_JOYSTICK_HIDAPI_XBOX "SDL_JOYSTICK_HIDAPI_XBOX"
#define SDL_HINT_JOYSTICK_LINUX_DIGITAL_HATS "SDL_JOYSTICK_LINUX_DIGITAL_HATS"
#ifndef SDL_HINT_JOYSTICK_IOKIT
#define SDL_HINT_JOYSTICK_IOKIT "SDL_JOYSTICK_IOKIT"
#endif
#ifndef SDL_HINT_JOYSTICK_MFI
#define SDL_HINT_JOYSTICK_MFI "SDL_JOYSTICK_MFI"
#endif
#define SDL_HINT_JOYSTICK_RAWINPUT SDL_HINT_JOYSTICK_RAWINPUT
#define SDL_HINT_JOYSTICK_WGI "SDL_JOYSTICK_WGI"
#define SDL_HINT_XINPUT_ENABLED "SDL_XINPUT_ENABLED"

// ============================================================================
// Log priority mapping
// ============================================================================
#define SDL_LOG_PRIORITY_TRACE SDL_LOG_PRIORITY_VERBOSE
#define SDL_LOG_PRIORITY_INVALID SDL_NUM_LOG_PRIORITIES
#define SDL_LOG_PRIORITY_COUNT SDL_NUM_LOG_PRIORITIES

// SDL3 renamed the log output function
#define SDL_SetLogOutputFunction SDL_LogSetOutputFunction
#define SDL_SetLogPriorities SDL_LogSetAllPriority

// ============================================================================
// Haptic constants
// ============================================================================
#ifndef SDL_MAX_RUMBLE_DURATION_MS
#define SDL_MAX_RUMBLE_DURATION_MS 0xFFFFFFFF
#endif

// SDL3 renamed some haptic functions
#define SDL_CreateHapticEffect SDL_HapticNewEffect
#define SDL_UpdateHapticEffect SDL_HapticUpdateEffect
#define SDL_RunHapticEffect SDL_HapticRunEffect
#define SDL_StopHapticEffect SDL_HapticStopEffect
#define SDL_DestroyHapticEffect SDL_HapticDestroyEffect
#define SDL_StopHapticRumble SDL_HapticRumbleStop
#define SDL_GetHapticFeatures SDL_HapticQuery

// ============================================================================
// Audio constants (for code that references them even though we use cubeb)
// ============================================================================
#define SDL_AUDIO_S16LE AUDIO_S16LSB
#define SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK 0

// ============================================================================
// Functions that changed return type from int (0=success) to bool (true=success)
// We save the original function pointers, then macro-redirect to bool wrappers.
// ============================================================================

// Helper: wrap an SDL2 function that returns 0 on success into a bool-returning one.
// We need to use function pointers to avoid macro recursion.

namespace SDL3Compat {

// Init functions
static inline bool Init(Uint32 flags) { return ::SDL_Init(flags) == 0; }
static inline bool InitSubSystem(Uint32 flags) { return ::SDL_InitSubSystem(flags) == 0; }

// Haptic functions
static inline bool InitHapticRumble(SDL_Haptic* h) { return ::SDL_HapticRumbleInit(h) == 0; }
static inline bool PlayHapticRumble(SDL_Haptic* h, float s, Uint32 l) { return ::SDL_HapticRumblePlay(h, s, l) == 0; }

// Screensaver
static inline bool DisableScreenSaver() { return ::SDL_DisableScreenSaver() == 0; }
static inline bool EnableScreenSaver() { return ::SDL_EnableScreenSaver() == 0; }

// Gamepad sensor
static inline bool SetGamepadSensorEnabled(SDL_GameController* gp, SDL_SensorType t, bool e)
{
  return ::SDL_GameControllerSetSensorEnabled(gp, t, e ? SDL_TRUE : SDL_FALSE) == 0;
}

// Gamepad rumble
static inline bool RumbleGamepad(SDL_GameController* gp, Uint16 lo, Uint16 hi, Uint32 dur)
{
  return ::SDL_GameControllerRumble(gp, lo, hi, dur) == 0;
}

// Gamepad LED
static inline bool SetGamepadLED(SDL_GameController* gp, Uint8 r, Uint8 g, Uint8 b)
{
  return ::SDL_GameControllerSetLED(gp, r, g, b) == 0;
}

// Gamepad effect
static inline bool SendGamepadEffect(SDL_GameController* gp, const void* data, int size)
{
  return ::SDL_GameControllerSendEffect(gp, data, size) == 0;
}

} // namespace SDL3Compat

// Redirect SDL3 names to our bool-returning wrappers.
// Must come AFTER the wrapper definitions and AFTER SDL2 header inclusion.
#undef SDL_Init
#define SDL_Init SDL3Compat::Init
#undef SDL_InitSubSystem
#define SDL_InitSubSystem SDL3Compat::InitSubSystem
#define SDL_InitHapticRumble SDL3Compat::InitHapticRumble
#define SDL_PlayHapticRumble SDL3Compat::PlayHapticRumble
#undef SDL_DisableScreenSaver
#define SDL_DisableScreenSaver SDL3Compat::DisableScreenSaver
#undef SDL_EnableScreenSaver
#define SDL_EnableScreenSaver SDL3Compat::EnableScreenSaver
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
#define SDL_GetGamepadName SDL_GameControllerGetName
#define SDL_GetGamepadPlayerIndex SDL_GameControllerGetPlayerIndex
#define SDL_GetGamepadVendor SDL_GameControllerGetVendor
#define SDL_GetGamepadProduct SDL_GameControllerGetProduct
#define SDL_IsGamepad SDL_IsGameController
#define SDL_GamepadHasSensor SDL_GameControllerHasSensor

// SDL3 SDL_GetGamepadType takes a gamepad pointer; SDL2's equivalent:
static inline SDL_GameControllerType SDL3_GetGamepadType(SDL_GameController* gp)
{
  return SDL_GameControllerGetType(gp);
}
#define SDL_GetGamepadType SDL3_GetGamepadType

// ============================================================================
// Gamepad button label function (SDL3 only - provide stub for SDL2)
// ============================================================================
static inline SDL_GamepadButtonLabel SDL3_GetGamepadButtonLabelForType(SDL_GameControllerType type,
                                                                       SDL_GameControllerButton button)
{
  // Map based on controller type - PS controllers use cross/circle/square/triangle
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
// SDL3: SDL_GetGamepadBindings(gp, &count) returns allocated array
// SDL2: SDL_GameControllerGetBindForAxis/Button for individual queries
// ============================================================================
static inline SDL_GameControllerButtonBind* SDL3_GetGamepadBindings(SDL_GameController* gp, int* count)
{
  // Count all bound axes and buttons
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
// SDL3: SDL_GetGamepadMappings(&count) returns allocated array of strings
// SDL2: SDL_GameControllerNumMappings() + SDL_GameControllerMappingForIndex()
// ============================================================================
static inline char** SDL3_GetGamepadMappings(int* count)
{
  *count = SDL_GameControllerNumMappings();
  // Caller just frees the result and uses count, so return a minimal allocation
  char** result = (char**)SDL_malloc(sizeof(char*));
  result[0] = nullptr;
  return result;
}
#define SDL_GetGamepadMappings SDL3_GetGamepadMappings

// ============================================================================
// Gamepad properties (SDL3 Properties API - stub for SDL2)
// ============================================================================
typedef Uint32 SDL_PropertiesID;

static inline SDL_PropertiesID SDL_GetGamepadProperties(SDL_GameController* /*gp*/) { return 0; }
static inline bool SDL_GetBooleanProperty(SDL_PropertiesID /*props*/, const char* /*name*/, bool def) { return def; }

// Property name constants used by DuckStation
#define SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN "SDL.gamepad.cap.rumble"
#define SDL_PROP_GAMEPAD_CAP_RGB_LED_BOOLEAN "SDL.gamepad.cap.rgb_led"
#define SDL_PROP_GAMEPAD_CAP_MONO_LED_BOOLEAN "SDL.gamepad.cap.mono_led"

// ============================================================================
// Joystick functions (minor renames in SDL3)
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

// SDL3: SDL_GetJoystickProperties() - stub returning 0
static inline SDL_PropertiesID SDL_GetJoystickProperties(SDL_Joystick* /*j*/) { return 0; }

// SDL3: SDL_GUIDToString writes to caller buffer; SDL2 does the same
#define SDL_GUIDToString SDL_JoystickGetGUIDString

// ============================================================================
// Window functions
// ============================================================================

// SDL3: SDL_CreateWindow(title, w, h, flags)
// SDL2: SDL_CreateWindow(title, x, y, w, h, flags)
static inline SDL_Window* SDL3_CreateWindow(const char* title, int w, int h, Uint32 flags)
{
  return ::SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
}
// Must undef first in case SDL2 defines it as a macro (it doesn't, but be safe)
#undef SDL_CreateWindow
#define SDL_CreateWindow SDL3_CreateWindow

// SDL3: SDL_GetWindowSizeInPixels(win, &w, &h)
// SDL2: SDL_GL_GetDrawableSize or SDL_GetWindowSize * scale
static inline void SDL3_GetWindowSizeInPixels(SDL_Window* win, int* w, int* h)
{
  // Try GL drawable size first, fall back to window size
  SDL_GL_GetDrawableSize(win, w, h);
  if (*w == 0 || *h == 0)
    SDL_GetWindowSize(win, w, h);
}
#define SDL_GetWindowSizeInPixels SDL3_GetWindowSizeInPixels

// SDL3: SDL_GetWindowDisplayScale(win) returns float
// SDL2: No direct equivalent; assume 1.0
static inline float SDL3_GetWindowDisplayScale(SDL_Window* /*win*/) { return 1.0f; }
#define SDL_GetWindowDisplayScale SDL3_GetWindowDisplayScale

// SDL3: SDL_GetDisplayForWindow(win) returns SDL_DisplayID
// SDL2: SDL_GetWindowDisplayIndex(win) returns int
typedef int SDL_DisplayID;
static inline SDL_DisplayID SDL3_GetDisplayForWindow(SDL_Window* win)
{
  int idx = SDL_GetWindowDisplayIndex(win);
  return (idx >= 0) ? (idx + 1) : 0; // Return 0 for invalid, 1-based otherwise
}
#define SDL_GetDisplayForWindow SDL3_GetDisplayForWindow

// SDL3: SDL_GetCurrentDisplayMode(displayID) returns const SDL_DisplayMode*
// SDL2: SDL_GetCurrentDisplayMode(displayIndex, &mode) fills struct
static inline const SDL_DisplayMode* SDL3_GetCurrentDisplayMode(SDL_DisplayID display_id)
{
  static thread_local SDL_DisplayMode s_mode;
  if (display_id > 0 && SDL_GetCurrentDisplayMode(display_id - 1, &s_mode) == 0)
    return &s_mode;
  return nullptr;
}
// Avoid conflict with SDL2's SDL_GetCurrentDisplayMode
#undef SDL_GetCurrentDisplayMode
#define SDL_GetCurrentDisplayMode SDL3_GetCurrentDisplayMode

// SDL3: SDL_GetWindowID(win) - same in both
// SDL3: SDL_GetCurrentVideoDriver() - same in both

// ============================================================================
// Window Properties API (SDL3 only - implement via SDL_GetWindowWMInfo for SDL2)
// ============================================================================

// Property name constants
#define SDL_PROP_WINDOW_X11_DISPLAY_POINTER "SDL.window.x11.display"
#define SDL_PROP_WINDOW_X11_WINDOW_NUMBER "SDL.window.x11.window"
#define SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER "SDL.window.wayland.display"
#define SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER "SDL.window.wayland.surface"

static inline SDL_PropertiesID SDL3_GetWindowProperties(SDL_Window* win)
{
  // Return the window pointer as a fake property ID (just needs to be non-zero)
  return (SDL_PropertiesID)(uintptr_t)win;
}
#define SDL_GetWindowProperties SDL3_GetWindowProperties

static inline void* SDL3_GetPointerProperty(SDL_PropertiesID props, const char* name, void* def)
{
  SDL_Window* win = (SDL_Window*)(uintptr_t)props;
  if (!win)
    return def;

  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (!SDL_GetWindowWMInfo(win, &info))
    return def;

#if defined(SDL_VIDEO_DRIVER_X11)
  if (std::strcmp(name, SDL_PROP_WINDOW_X11_DISPLAY_POINTER) == 0)
    return (void*)info.info.x11.display;
#endif
#if defined(SDL_VIDEO_DRIVER_WAYLAND)
  if (std::strcmp(name, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER) == 0)
    return (void*)info.info.wl.display;
  if (std::strcmp(name, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER) == 0)
    return (void*)info.info.wl.surface;
#endif

  return def;
}
#define SDL_GetPointerProperty SDL3_GetPointerProperty

static inline Sint64 SDL3_GetNumberProperty(SDL_PropertiesID props, const char* name, Sint64 def)
{
  SDL_Window* win = (SDL_Window*)(uintptr_t)props;
  if (!win)
    return def;

  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (!SDL_GetWindowWMInfo(win, &info))
    return def;

#if defined(SDL_VIDEO_DRIVER_X11)
  if (std::strcmp(name, SDL_PROP_WINDOW_X11_WINDOW_NUMBER) == 0)
    return (Sint64)info.info.x11.window;
#endif

  return def;
}
#define SDL_GetNumberProperty SDL3_GetNumberProperty

// ============================================================================
// Mouse / cursor functions (SDL3 renames)
// ============================================================================
static inline bool SDL3_SetWindowRelativeMouseMode(SDL_Window* /*win*/, bool enabled)
{
  return SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE) == 0;
}
#define SDL_SetWindowRelativeMouseMode SDL3_SetWindowRelativeMouseMode

static inline void SDL3_HideCursor() { SDL_ShowCursor(SDL_DISABLE); }
static inline void SDL3_ShowCursor() { SDL_ShowCursor(SDL_ENABLE); }
#undef SDL_HideCursor
#define SDL_HideCursor SDL3_HideCursor
#undef SDL_ShowCursor
#define SDL_ShowCursor SDL3_ShowCursor

// ============================================================================
// Text input (SDL3 takes a window parameter)
// ============================================================================
static inline void SDL3_StartTextInput(SDL_Window* /*win*/) { ::SDL_StartTextInput(); }
static inline void SDL3_StopTextInput(SDL_Window* /*win*/) { ::SDL_StopTextInput(); }
#undef SDL_StartTextInput
#define SDL_StartTextInput SDL3_StartTextInput
#undef SDL_StopTextInput
#define SDL_StopTextInput SDL3_StopTextInput

// ============================================================================
// Clipboard (same API in SDL2 and SDL3)
// ============================================================================
// SDL_GetClipboardText, SDL_SetClipboardText - same signatures

// ============================================================================
// Misc functions
// ============================================================================
// SDL_OpenURL - same in both
// SDL_ShowSimpleMessageBox - same in both
// SDL_GetError - same in both
// SDL_free - same in both
// SDL_PollEvent - same in both
// SDL_WaitEventTimeout - same in both (but SDL3 returns bool; SDL2 returns int - truthiness is same)
// SDL_SetHint - same in both
// SDL_Quit - same in both
// SDL_QuitSubSystem - same in both (but flag names differ - already handled above)

// SDL3: SDL_STANDARD_GRAVITY
#ifndef SDL_STANDARD_GRAVITY
#define SDL_STANDARD_GRAVITY 9.80665f
#endif

// SDL3: SDL_DEFAULT_XBOX_HIDAPI - not in SDL2, define as empty/zero
#ifndef SDL_DEFAULT_XBOX_HIDAPI
#define SDL_DEFAULT_XBOX_HIDAPI 0
#endif

// SDL3: SDL_hidapi_ps5 - not in SDL2
#ifndef SDL_hidapi_ps5
#define SDL_hidapi_ps5 0
#endif

// ============================================================================
// Keyboard event compatibility
// SDL3: event.key.scancode, event.key.repeat (bool)
// SDL2: event.key.keysym.scancode, event.key.repeat (Uint8)
// This can't be fixed by a header alone - the frontend code must handle this.
// The sdl_input_source does NOT access keyboard events, only the frontend does.
// ============================================================================
