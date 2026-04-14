// SPDX-FileCopyrightText: 2019-2026 Connor McLaughlin <stenzek@gmail.com>
// SPDX-License-Identifier: CC-BY-NC-ND-4.0

#include "core/achievements.h"
#include "core/bus.h"
#include "core/controller.h"
#include "core/core.h"
#include "core/core_private.h"
#include "core/cpu_core.h"
#include "core/fullscreenui.h"
#include "core/game_list.h"
#include "core/gpu_backend.h"
#include "core/host.h"
#include "core/settings.h"
#include "core/system.h"
#include "core/system_private.h"
#include "core/video_thread.h"

#include "scmversion/scmversion.h"

#include "util/audio_stream.h"
#include "util/gpu_device.h"
#include "util/imgui_manager.h"
#include "util/input_manager.h"
#include "util/window_info.h"
#include "util/sdl_input_source.h"
#include "util/translation.h"

#include "common/assert.h"
#include "common/crash_handler.h"
#include "common/error.h"
#include "common/file_system.h"
#include "common/log.h"
#include "common/path.h"
#include "common/string_util.h"
#include "common/task_queue.h"
#include "common/threading.h"
#include "common/time_helpers.h"

#include "fmt/format.h"

#include <SDL3/SDL.h>
#include <SDL_syswm.h>

#include <csignal>
#include <cstdio>
#include <cstring>

LOG_CHANNEL(Host);

// Stub for SDL audio stream (we use cubeb instead).
// The symbol is needed because audio_stream.cpp references it for AudioBackend::SDL.
std::unique_ptr<AudioStream> AudioStream::CreateSDLAudioStream(u32, u32, u32, bool, AudioStreamSource*, bool,
                                                               Error* error)
{
  Error::SetStringView(error, "SDL audio backend is not available in this build. Use Cubeb.");
  return {};
}

namespace SDLHost {

static bool ParseCommandLineParameters(int argc, char* argv[], std::optional<SystemBootParameters>& autoboot);
static void PrintCommandLineHelp(const char* progname);
static bool InitializeFoldersAndConfig(Error* error);
static void HookSignals();
static void ProcessCoreThreadEvents();
static void PumpSDLEvents();
static void ProcessSDLEvent(const SDL_Event* event);
static void VideoThreadEntryPoint();
static bool CreateSDLWindow(bool fullscreen);
static void DestroySDLWindow();
static void FillWindowInfo(WindowInfo* wi);

struct SDLHostState
{
  ALIGN_TO_CACHE_LINE std::mutex core_thread_events_mutex;
  std::condition_variable core_thread_event_done;
  std::deque<std::pair<std::function<void()>, bool>> cpu_thread_events;
  u32 blocking_cpu_events_pending = 0;
};

static SDLHostState s_state;
ALIGN_TO_CACHE_LINE static TaskQueue s_async_task_queue;

static SDL_Window* s_sdl_window = nullptr;
static bool s_shutdown_flag = false;
static bool s_start_fullscreen = true;
static bool s_relative_mouse_enabled = false;

} // namespace SDLHost

static Threading::Thread s_video_thread;

// ------------------------------------------------------------------------------------------------
// Initialization
// ------------------------------------------------------------------------------------------------

bool SDLHost::InitializeFoldersAndConfig(Error* error)
{
  if (!Core::SetCriticalFolders("resources", error))
    return false;

  if (!Core::InitializeBaseSettingsLayer({}, error))
    return false;

  // Set sensible defaults for handheld/standalone use.
  const auto lock = Core::GetSettingsLock();
  SettingsInterface& si = *Core::GetBaseSettingsLayer();

  // Only set defaults if settings file is fresh (no renderer configured yet).
  if (si.GetStringValue("GPU", "Renderer", "").empty())
  {
    si.SetStringValue("GPU", "Renderer", Settings::GetRendererName(GPURenderer::AutoDetect));
    si.SetStringValue("Audio", "Backend", AudioStream::GetBackendName(AudioBackend::SDL));
    si.SetStringValue("Pad1", "Type", Controller::GetControllerInfo(ControllerType::AnalogController).name);
    si.SetStringValue("Pad2", "Type", Controller::GetControllerInfo(ControllerType::None).name);
    si.SetStringValue("MemoryCards", "Card1Type", Settings::GetMemoryCardTypeName(MemoryCardType::PerGameTitle));
    si.SetStringValue("MemoryCards", "Card2Type", Settings::GetMemoryCardTypeName(MemoryCardType::None));
    si.SetBoolValue("BIOS", "PatchFastBoot", true);
    si.SetBoolValue("Main", "StartFullscreen", true);

    // Enable SDL input source.
    si.SetBoolValue("InputSources", InputManager::InputSourceToString(InputSourceType::SDL), true);
  }

  return true;
}

// ------------------------------------------------------------------------------------------------
// SDL Window Management
// ------------------------------------------------------------------------------------------------

bool SDLHost::CreateSDLWindow(bool fullscreen)
{
  u32 flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
  if (fullscreen)
    flags |= SDL_WINDOW_FULLSCREEN;

  s_sdl_window = SDL_CreateWindow("DuckStation", 1280, 720, flags);
  if (!s_sdl_window)
  {
    ERROR_LOG("SDL_CreateWindow() failed: {}", SDL_GetError());
    return false;
  }

  return true;
}

void SDLHost::DestroySDLWindow()
{
  if (s_sdl_window)
  {
    SDL_DestroyWindow(s_sdl_window);
    s_sdl_window = nullptr;
  }
}

void SDLHost::FillWindowInfo(WindowInfo* wi)
{
  *wi = {};

  if (!s_sdl_window)
    return;

  const char* video_driver = SDL_GetCurrentVideoDriver();
  const SDL_PropertiesID props = SDL_GetWindowProperties(s_sdl_window);

#ifdef __linux__
  if (video_driver && std::strcmp(video_driver, "wayland") == 0)
  {
    wi->type = WindowInfoType::Wayland;
    wi->display_connection = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
    wi->window_handle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
  }
  else if (video_driver && std::strcmp(video_driver, "x11") == 0)
  {
    wi->type = WindowInfoType::XCB;
    wi->display_connection = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
    // X11 Window is a long, stored as a number property.
    wi->window_handle =
      reinterpret_cast<void*>(static_cast<uintptr_t>(SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0)));
  }
  else
#endif
  {
    wi->type = WindowInfoType::Surfaceless;
  }

  int w, h;
  SDL_GetWindowSizeInPixels(s_sdl_window, &w, &h);
  wi->surface_width = static_cast<u16>(w);
  wi->surface_height = static_cast<u16>(h);
  wi->surface_scale = SDL_GetWindowDisplayScale(s_sdl_window);

  const SDL_DisplayID display_id = SDL_GetDisplayForWindow(s_sdl_window);
  if (display_id)
  {
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display_id);
    if (mode)
      wi->surface_refresh_rate = mode->refresh_rate;
  }
}

// ------------------------------------------------------------------------------------------------
// SDL Event Handling
// ------------------------------------------------------------------------------------------------

void SDLHost::PumpSDLEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
    ProcessSDLEvent(&event);
}

void SDLHost::ProcessSDLEvent(const SDL_Event* event)
{
  // Let SDLInputSource handle controller events.
  if (SDLInputSource::IsHandledInputEvent(event))
    return;

  switch (event->type)
  {
    case SDL_EVENT_QUIT:
      s_shutdown_flag = true;
      break;

    case SDL_WINDOWEVENT:
    {
      if (event->window.event == SDL_WINDOWEVENT_RESIZED &&
          event->window.windowID == SDL_GetWindowID(s_sdl_window))
      {
        int w, h;
        SDL_GetWindowSizeInPixels(s_sdl_window, &w, &h);
        const float scale = SDL_GetWindowDisplayScale(s_sdl_window);

        float refresh_rate = 0.0f;
        const SDL_DisplayID display_id = SDL_GetDisplayForWindow(s_sdl_window);
        if (display_id)
        {
          const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display_id);
          if (mode)
            refresh_rate = mode->refresh_rate;
        }

        VideoThread::ResizeRenderWindow(w, h, scale, refresh_rate);
      }
      break;
    }

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
    {
      // SDL2: scancode is at event->key.keysym.scancode
      if (!event->key.repeat)
      {
        const u32 scancode = static_cast<u32>(event->key.keysym.scancode);
        const bool pressed = (event->type == SDL_EVENT_KEY_DOWN);
        InputManager::InvokeEvents(InputManager::MakeHostKeyboardKey(scancode), pressed ? 1.0f : 0.0f);
      }
      break;
    }

    case SDL_EVENT_MOUSE_MOTION:
    {
      // SDL2: xrel/yrel are Sint32, cast to float
      if (s_relative_mouse_enabled)
      {
        InputManager::UpdatePointerPositionRelativeDelta(0, InputPointerAxis::X, static_cast<float>(event->motion.xrel));
        InputManager::UpdatePointerPositionRelativeDelta(0, InputPointerAxis::Y, static_cast<float>(event->motion.yrel));
      }
      else
      {
        InputManager::UpdatePointerAbsolutePosition(0, static_cast<float>(event->motion.x),
                                                    static_cast<float>(event->motion.y));
      }
      break;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
      // SDL mouse buttons are 1-indexed.
      const u32 button_index = event->button.button - 1;
      const bool pressed = (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN);
      InputManager::InvokeEvents(InputManager::MakePointerButtonKey(0, button_index), pressed ? 1.0f : 0.0f);
      break;
    }

    case SDL_EVENT_MOUSE_WHEEL:
    {
      // SDL2: wheel x/y are Sint32
      if (event->wheel.y != 0)
      {
        InputManager::InvokeEvents(InputManager::MakePointerAxisKey(0, InputPointerAxis::WheelY),
                                   static_cast<float>(event->wheel.y));
      }
      if (event->wheel.x != 0)
      {
        InputManager::InvokeEvents(InputManager::MakePointerAxisKey(0, InputPointerAxis::WheelX),
                                   static_cast<float>(event->wheel.x));
      }
      break;
    }

    case SDL_EVENT_TEXT_INPUT:
    {
      if (event->text.text)
        ImGuiManager::AddTextInput(std::string(event->text.text));
      break;
    }

    default:
      break;
  }
}

// ------------------------------------------------------------------------------------------------
// Core Thread Events (same pattern as regtest)
// ------------------------------------------------------------------------------------------------

void SDLHost::ProcessCoreThreadEvents()
{
  std::unique_lock lock(s_state.core_thread_events_mutex);

  for (;;)
  {
    if (s_state.cpu_thread_events.empty())
      break;

    auto event = std::move(s_state.cpu_thread_events.front());
    s_state.cpu_thread_events.pop_front();
    lock.unlock();
    event.first();
    lock.lock();

    if (event.second)
    {
      s_state.blocking_cpu_events_pending--;
      s_state.core_thread_event_done.notify_one();
    }
  }
}

// ------------------------------------------------------------------------------------------------
// Host Interface Implementation
// ------------------------------------------------------------------------------------------------

void Host::ReportFatalError(std::string_view title, std::string_view message)
{
  ERROR_LOG("ReportFatalError: {}", message);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, SmallString(title).c_str(), SmallString(message).c_str(),
                           SDLHost::s_sdl_window);
  abort();
}

void Host::ReportErrorAsync(std::string_view title, std::string_view message)
{
  if (!title.empty() && !message.empty())
    ERROR_LOG("ReportErrorAsync: {}: {}", title, message);
  else if (!message.empty())
    ERROR_LOG("ReportErrorAsync: {}", message);
}

void Host::ReportStatusMessage(std::string_view message)
{
  INFO_LOG("ReportStatusMessage: {}", message);
}

void Host::ConfirmMessageAsync(std::string_view title, std::string_view message, ConfirmMessageAsyncCallback callback,
                               std::string_view yes_text, std::string_view no_text)
{
  if (!title.empty() && !message.empty())
    INFO_LOG("ConfirmMessage: {}: {}", title, message);
  else if (!message.empty())
    INFO_LOG("ConfirmMessage: {}", message);

  callback(true);
}

void Host::ReportDebuggerEvent(CPU::DebuggerEvent event, std::string_view message)
{
  ERROR_LOG("ReportDebuggerEvent: {}", message);
}

std::span<const std::pair<const char*, const char*>> Host::GetAvailableLanguageList()
{
  return {};
}

const char* Host::GetLanguageName(std::string_view language_code)
{
  return "";
}

bool Host::ChangeLanguage(const char* new_language)
{
  return false;
}

s32 Host::Internal::GetTranslatedStringImpl(std::string_view context, std::string_view msg,
                                            std::string_view disambiguation, char* tbuf, size_t tbuf_space)
{
  if (msg.size() > tbuf_space)
    return -1;
  else if (msg.empty())
    return 0;

  std::memcpy(tbuf, msg.data(), msg.size());
  return static_cast<s32>(msg.size());
}

std::string Host::TranslatePluralToString(const char* context, const char* msg, const char* disambiguation, int count)
{
  TinyString count_str = TinyString::from_format("{}", count);

  std::string ret(msg);
  for (;;)
  {
    std::string::size_type pos = ret.find("%n");
    if (pos == std::string::npos)
      break;
    ret.replace(pos, pos + 2, count_str.view());
  }

  return ret;
}

SmallString Host::TranslatePluralToSmallString(const char* context, const char* msg, const char* disambiguation,
                                               int count)
{
  SmallString ret(msg);
  ret.replace("%n", TinyString::from_format("{}", count));
  return ret;
}

void Host::LoadSettings(const SettingsInterface& si, std::unique_lock<std::mutex>& lock)
{
  // Nothing host-specific to load.
}

void Host::CheckForSettingsChanges(const Settings& old_settings)
{
  // Nothing host-specific to check.
}

void Host::CommitBaseSettingChanges()
{
  Error error;
  if (!Core::SaveBaseSettingsLayer(&error))
    ERROR_LOG("Failed to save settings: {}", error.GetDescription());
}

bool Host::ResourceFileExists(std::string_view filename, bool allow_override)
{
  const std::string path(Path::Combine(EmuFolders::Resources, filename));
  return FileSystem::FileExists(path.c_str());
}

std::optional<DynamicHeapArray<u8>> Host::ReadResourceFile(std::string_view filename, bool allow_override, Error* error)
{
  const std::string path(Path::Combine(EmuFolders::Resources, filename));
  return FileSystem::ReadBinaryFile(path.c_str(), error);
}

std::optional<std::string> Host::ReadResourceFileToString(std::string_view filename, bool allow_override, Error* error)
{
  const std::string path(Path::Combine(EmuFolders::Resources, filename));
  return FileSystem::ReadFileToString(path.c_str(), error);
}

std::optional<std::time_t> Host::GetResourceFileTimestamp(std::string_view filename, bool allow_override)
{
  const std::string path(Path::Combine(EmuFolders::Resources, filename));
  FILESYSTEM_STAT_DATA sd;
  if (!FileSystem::StatFile(path.c_str(), &sd))
    return std::nullopt;
  return sd.ModificationTime;
}

void Host::OnSystemStarting()
{
}

void Host::OnSystemStarted()
{
}

void Host::OnSystemStopping()
{
}

void Host::OnSystemDestroyed()
{
}

void Host::OnSystemPaused()
{
}

void Host::OnSystemResumed()
{
}

void Host::OnSystemAbnormalShutdown(const std::string_view reason)
{
}

void Host::OnVideoThreadRunIdleChanged(bool is_active)
{
}

bool Host::SetScreensaverInhibit(bool inhibit, Error* error)
{
  if (inhibit)
    return SDL_DisableScreenSaver();
  else
    return SDL_EnableScreenSaver();
}

void Host::OnPerformanceCountersUpdated(const GPUBackend* gpu_backend)
{
}

void Host::OnSystemGameChanged(const std::string& disc_path, const std::string& game_serial,
                               const std::string& game_name, GameHash hash)
{
  INFO_LOG("Game: {} ({})", game_name, game_serial);
}

void Host::OnSystemUndoStateAvailabilityChanged(bool available, u64 timestamp)
{
}

void Host::OnMediaCaptureStarted()
{
}

void Host::OnMediaCaptureStopped()
{
}

void Host::PumpMessagesOnCoreThread()
{
  SDLHost::PumpSDLEvents();
  SDLHost::ProcessCoreThreadEvents();
}

void Host::RunOnCoreThread(std::function<void()> function, bool block /* = false */)
{
  using namespace SDLHost;

  std::unique_lock lock(s_state.core_thread_events_mutex);
  s_state.cpu_thread_events.emplace_back(std::move(function), block);
  s_state.blocking_cpu_events_pending += BoolToUInt32(block);
  if (block)
    s_state.core_thread_event_done.wait(lock, []() { return s_state.blocking_cpu_events_pending == 0; });
}

void Host::RunOnUIThread(std::function<void()> function, bool block /* = false */)
{
  RunOnCoreThread(std::move(function), block);
}

void Host::QueueAsyncTask(std::function<void()> function)
{
  SDLHost::s_async_task_queue.SubmitTask(std::move(function));
}

void Host::WaitForAllAsyncTasks()
{
  SDLHost::s_async_task_queue.WaitForAll();
}

void Host::RequestResizeHostDisplay(s32 width, s32 height)
{
}

void Host::SetDefaultSettings(SettingsInterface& si)
{
}

void Host::OnSettingsResetToDefault(bool host, bool system, bool controller)
{
}

void Host::RequestExitApplication(bool save_state_if_running)
{
  if (System::IsValid() && save_state_if_running)
    System::ShutdownSystem(true);

  SDLHost::s_shutdown_flag = true;
}

void Host::RequestExitBigPicture()
{
  // On handheld, exiting Big Picture exits the application.
  RequestExitApplication(true);
}

void Host::RequestSystemShutdown(bool allow_confirm, bool save_state, bool check_memcard_busy)
{
  Host::RunOnCoreThread([save_state]() {
    if (System::IsValid())
      System::ShutdownSystem(save_state);
  });
}

std::optional<WindowInfo> Host::AcquireRenderWindow(RenderAPI render_api, bool fullscreen, bool exclusive_fullscreen,
                                                    Error* error)
{
  // Destroy existing window if switching modes.
  SDLHost::DestroySDLWindow();

  if (!SDLHost::CreateSDLWindow(fullscreen))
  {
    Error::SetStringView(error, "Failed to create SDL window");
    return std::nullopt;
  }

  WindowInfo wi;
  SDLHost::FillWindowInfo(&wi);

  INFO_LOG("Render window: {}x{}, scale: {}, refresh: {} Hz, type: {}",
           wi.surface_width, wi.surface_height, wi.surface_scale, wi.surface_refresh_rate,
           static_cast<int>(wi.type));

  return wi;
}

WindowInfoType Host::GetRenderWindowInfoType()
{
#ifdef __linux__
  const char* video_driver = SDL_GetCurrentVideoDriver();
  if (video_driver && std::strcmp(video_driver, "wayland") == 0)
    return WindowInfoType::Wayland;
  else if (video_driver && std::strcmp(video_driver, "x11") == 0)
    return WindowInfoType::XCB;
#endif
  return WindowInfoType::Surfaceless;
}

void Host::ReleaseRenderWindow()
{
  SDLHost::DestroySDLWindow();
}

bool Host::CanChangeFullscreenMode(bool new_fullscreen_state)
{
  return true;
}

void Host::BeginTextInput()
{
  if (SDLHost::s_sdl_window)
    SDL_StartTextInput(SDLHost::s_sdl_window);
}

void Host::EndTextInput()
{
  if (SDLHost::s_sdl_window)
    SDL_StopTextInput(SDLHost::s_sdl_window);
}

bool Host::CreateAuxiliaryRenderWindow(s32 x, s32 y, u32 width, u32 height, std::string_view title,
                                       std::string_view icon_name, AuxiliaryRenderWindowUserData userdata,
                                       AuxiliaryRenderWindowHandle* handle, WindowInfo* wi, Error* error)
{
  return false;
}

void Host::DestroyAuxiliaryRenderWindow(AuxiliaryRenderWindowHandle handle, s32* pos_x, s32* pos_y, u32* width,
                                        u32* height)
{
}

void Host::FrameDoneOnVideoThread(GPUBackend* gpu_backend, u32 frame_number)
{
}

void Host::OpenURL(std::string_view url)
{
  SDL_OpenURL(SmallString(url).c_str());
}

std::string Host::GetClipboardText()
{
  char* text = SDL_GetClipboardText();
  std::string result;
  if (text)
  {
    result = text;
    SDL_free(text);
  }
  return result;
}

bool Host::CopyTextToClipboard(std::string_view text)
{
  return SDL_SetClipboardText(SmallString(text).c_str());
}

std::string Host::FormatNumber(NumberFormatType type, s64 value)
{
  std::string ret;

  if (type >= NumberFormatType::ShortDate && type <= NumberFormatType::LongDateTime)
  {
    const char* format;
    switch (type)
    {
      case NumberFormatType::ShortDate:
        format = "%x";
        break;
      case NumberFormatType::LongDate:
        format = "%A %B %e %Y";
        break;
      case NumberFormatType::ShortTime:
      case NumberFormatType::LongTime:
        format = "%X";
        break;
      case NumberFormatType::ShortDateTime:
        format = "%X %x";
        break;
      case NumberFormatType::LongDateTime:
        format = "%c";
        break;
        DefaultCaseIsUnreachable();
    }

    ret.resize(128);
    if (const std::optional<std::tm> ltime = Common::LocalTime(static_cast<std::time_t>(value)))
      ret.resize(std::strftime(ret.data(), ret.size(), format, &ltime.value()));
    else
      ret = "Invalid";
  }
  else
  {
    ret = fmt::format("{}", value);
  }

  return ret;
}

std::string Host::FormatNumber(NumberFormatType type, double value)
{
  return fmt::format("{}", value);
}

void Host::SetMouseMode(bool relative, bool hide_cursor)
{
  SDLHost::s_relative_mouse_enabled = relative;
  if (SDLHost::s_sdl_window)
  {
    SDL_SetWindowRelativeMouseMode(SDLHost::s_sdl_window, relative);
    if (hide_cursor)
      SDL_HideCursor();
    else
      SDL_ShowCursor();
  }
}

void Host::OnAchievementsLoginRequested(Achievements::LoginRequestReason reason)
{
}

void Host::OnAchievementsLoginSuccess(const char* username, u32 points, u32 sc_points, u32 unread_messages)
{
}

void Host::OnAchievementsActiveChanged(bool active)
{
}

void Host::OnAchievementsHardcoreModeChanged(bool enabled)
{
}

#ifdef RC_CLIENT_SUPPORTS_RAINTEGRATION

void Host::OnRAIntegrationMenuChanged()
{
}

#endif

const char* Host::GetDefaultFullscreenUITheme()
{
  return "";
}

void Host::AddFixedInputBindings(const SettingsInterface& si)
{
}

void Host::OnInputDeviceConnected(InputBindingKey key, std::string_view identifier, std::string_view device_name)
{
  INFO_LOG("Input device connected: {} ({})", identifier, device_name);
}

void Host::OnInputDeviceDisconnected(InputBindingKey key, std::string_view identifier)
{
  INFO_LOG("Input device disconnected: {}", identifier);
}

std::optional<WindowInfo> Host::GetTopLevelWindowInfo()
{
  if (!SDLHost::s_sdl_window)
    return std::nullopt;

  WindowInfo wi;
  SDLHost::FillWindowInfo(&wi);
  return wi;
}

void Host::RefreshGameListAsync(bool invalidate_cache)
{
  Host::QueueAsyncTask([invalidate_cache]() { GameList::Refresh(invalidate_cache); });
}

void Host::CancelGameListRefresh()
{
  GameList::CancelRefresh();
}

void Host::OnGameListEntriesChanged(std::span<const u32> changed_indices)
{
}

// ------------------------------------------------------------------------------------------------
// Signals
// ------------------------------------------------------------------------------------------------

static void SignalHandler(int signal)
{
  std::signal(signal, SIG_DFL);
  std::quick_exit(1);
}

void SDLHost::HookSignals()
{
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);

#ifndef _WIN32
  struct sigaction sa_chld = {};
  sigemptyset(&sa_chld.sa_mask);
  sa_chld.sa_handler = SIG_IGN;
  sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_NOCLDWAIT;
  sigaction(SIGCHLD, &sa_chld, nullptr);
#endif
}

// ------------------------------------------------------------------------------------------------
// Video Thread
// ------------------------------------------------------------------------------------------------

void SDLHost::VideoThreadEntryPoint()
{
  Threading::SetNameOfCurrentThread("Video Thread");
  VideoThread::Internal::VideoThreadEntryPoint();
}

// ------------------------------------------------------------------------------------------------
// Command Line
// ------------------------------------------------------------------------------------------------

void SDLHost::PrintCommandLineHelp(const char* progname)
{
  std::fprintf(stderr, "DuckStation SDL Standalone Frontend\n");
  std::fprintf(stderr, "Usage: %s [options] [--] [boot filename]\n\n", progname);
  std::fprintf(stderr, "  -help: Displays this information and exits.\n");
  std::fprintf(stderr, "  -fullscreen: Start in fullscreen mode (default).\n");
  std::fprintf(stderr, "  -windowed: Start in windowed mode.\n");
  std::fprintf(stderr, "  -renderer <renderer>: Sets the GPU renderer.\n");
  std::fprintf(stderr, "  -portable: Use portable mode (settings next to binary).\n");
  std::fprintf(stderr, "  -log <level>: Sets the log level.\n");
  std::fprintf(stderr, "  --: Remaining arguments are the boot filename.\n\n");
}

bool SDLHost::ParseCommandLineParameters(int argc, char* argv[], std::optional<SystemBootParameters>& autoboot)
{
  bool no_more_args = false;
  for (int i = 1; i < argc; i++)
  {
    if (!no_more_args)
    {
#define CHECK_ARG(str) !std::strcmp(argv[i], str)
#define CHECK_ARG_PARAM(str) (!std::strcmp(argv[i], str) && ((i + 1) < argc))

      if (CHECK_ARG("-help"))
      {
        PrintCommandLineHelp(argv[0]);
        return false;
      }
      else if (CHECK_ARG("-fullscreen"))
      {
        s_start_fullscreen = true;
        continue;
      }
      else if (CHECK_ARG("-windowed"))
      {
        s_start_fullscreen = false;
        continue;
      }
      else if (CHECK_ARG_PARAM("-renderer"))
      {
        std::optional<GPURenderer> renderer = Settings::ParseRendererName(argv[++i]);
        if (!renderer.has_value())
        {
          ERROR_LOG("Invalid renderer specified.");
          return false;
        }
        Core::SetBaseStringSettingValue("GPU", "Renderer", Settings::GetRendererName(renderer.value()));
        continue;
      }
      else if (CHECK_ARG_PARAM("-log"))
      {
        std::optional<Log::Level> level = Settings::ParseLogLevelName(argv[++i]);
        if (!level.has_value())
        {
          ERROR_LOG("Invalid log level specified.");
          return false;
        }
        Log::SetLogLevel(level.value());
        Core::SetBaseStringSettingValue("Logging", "LogLevel", Settings::GetLogLevelName(level.value()));
        continue;
      }
      else if (CHECK_ARG("-portable"))
      {
        // Portable mode is handled by EmuFolders before we get here, but accept the flag.
        continue;
      }
      else if (CHECK_ARG("--"))
      {
        no_more_args = true;
        continue;
      }
      else if (argv[i][0] == '-')
      {
        ERROR_LOG("Unknown parameter: '{}'", argv[i]);
        return false;
      }

#undef CHECK_ARG
#undef CHECK_ARG_PARAM
    }

    if (!autoboot)
      autoboot.emplace();
    if (!autoboot->path.empty())
      autoboot->path += ' ';
    autoboot->path += argv[i];
  }

  return true;
}

// ------------------------------------------------------------------------------------------------
// Main
// ------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  CrashHandler::Install(&Bus::CleanupMemoryMap);

  Error error;
  if (!System::PerformEarlyHardwareChecks(&error) || !System::ProcessStartup(&error))
  {
    std::fprintf(stderr, "ERROR: Startup failed: %s\n", error.GetDescription().c_str());
    return EXIT_FAILURE;
  }

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC))
  {
    std::fprintf(stderr, "ERROR: SDL_Init() failed: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  // Disable SDLInputSource event polling — we pump events ourselves in PumpMessagesOnCoreThread.
  SDLInputSource::ALLOW_EVENT_POLLING = false;

  if (!SDLHost::InitializeFoldersAndConfig(&error))
  {
    std::fprintf(stderr, "ERROR: Failed to initialize config: %s\n", error.GetDescription().c_str());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  std::optional<SystemBootParameters> autoboot;
  if (!SDLHost::ParseCommandLineParameters(argc, argv, autoboot))
  {
    SDL_Quit();
    return EXIT_FAILURE;
  }

  if (!System::CoreThreadInitialize(&error))
  {
    ERROR_LOG("CoreThreadInitialize() failed: {}", error.GetDescription());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDLHost::s_async_task_queue.SetWorkerCount(2);
  SDLHost::HookSignals();

  // Start video thread.
  s_video_thread.Start(&SDLHost::VideoThreadEntryPoint);

  // Load settings and apply them.
  System::LoadSettings(false);

  int result = EXIT_FAILURE;

  // If a game was specified on the command line, boot it directly.
  if (autoboot && !autoboot->path.empty())
  {
    if (!System::BootSystem(std::move(autoboot.value()), &error))
    {
      ERROR_LOG("Failed to boot system: {}", error.GetDescription());
      goto cleanup;
    }
  }
  else
  {
    // Otherwise, start in FullscreenUI (Big Picture mode).
    if (!VideoThread::StartFullscreenUI(SDLHost::s_start_fullscreen, &error))
    {
      ERROR_LOG("Failed to start FullscreenUI: {}", error.GetDescription());
      goto cleanup;
    }
  }

  // Main loop — mirrors Qt's CoreThread::run().
  while (!SDLHost::s_shutdown_flag)
  {
    if (System::IsRunning())
    {
      System::Execute();
    }
    else if (!VideoThread::IsUsingThread() && VideoThread::IsRunningIdle())
    {
      SDLHost::PumpSDLEvents();
      SDLHost::ProcessCoreThreadEvents();

      if (!VideoThread::IsUsingThread() && VideoThread::IsRunningIdle())
        VideoThread::Internal::DoRunIdle();
    }
    else
    {
      // Idle wait — save power on handhelds.
      SDL_Event event;
      if (SDL_WaitEventTimeout(&event, 50))
        SDLHost::ProcessSDLEvent(&event);
      SDLHost::ProcessCoreThreadEvents();
    }
  }

  if (System::IsValid())
    System::ShutdownSystem(false);

  result = EXIT_SUCCESS;

cleanup:
  if (s_video_thread.Joinable())
  {
    VideoThread::Internal::RequestShutdown();
    s_video_thread.Join();
  }

  SDLHost::s_async_task_queue.SetWorkerCount(0);
  SDLHost::ProcessCoreThreadEvents();
  System::CoreThreadShutdown();
  System::ProcessShutdown();
  SDLHost::DestroySDLWindow();
  SDL_Quit();
  return result;
}
