// SPDX-FileCopyrightText: 2021-2026 Connor McLaughlin <stenzek@gmail.com>
// SPDX-License-Identifier: CC-BY-NC-ND-4.0
// Ported from the original DuckStation DRM/GBM support, adapted to current EGL context API.

#pragma once

#include "drm_display.h"
#include "opengl_context_egl.h"

#include <array>
#include <atomic>
#include <condition_variable>
#include <gbm.h>
#include <mutex>
#include <thread>

class OpenGLContextEGLGBM final : public OpenGLContextEGL
{
public:
  OpenGLContextEGLGBM();
  ~OpenGLContextEGLGBM() override;

  static std::unique_ptr<OpenGLContext> Create(WindowInfo& wi, SurfaceHandle* surface,
                                               std::span<const Version> versions_to_try, Error* error);

  std::unique_ptr<OpenGLContext> CreateSharedContext(WindowInfo& wi, SurfaceHandle* surface, Error* error) override;
  void ResizeSurface(WindowInfo& wi, SurfaceHandle handle) override;
  bool SwapBuffers() override;
  bool SetSwapInterval(s32 interval, Error* error = nullptr) override;

protected:
  EGLDisplay GetPlatformDisplay(const WindowInfo& wi, Error* error) override;
  EGLSurface CreatePlatformSurface(EGLConfig config, const WindowInfo& wi, Error* error) override;

private:
  enum : u32
  {
    MAX_BUFFERS = 5
  };

  struct Buffer
  {
    struct gbm_bo* bo;
    u32 fb_id;
  };

  DRMDisplay* GetDRM() { return static_cast<DRMDisplay*>(m_drm_display); }

  bool CreateGBMDevice();
  Buffer* LockFrontBuffer();
  void ReleaseBuffer(Buffer* buffer);
  void PresentBuffer(Buffer* buffer, bool wait_for_vsync);

  void StartPresentThread();
  void StopPresentThread();
  void PresentThread();

  bool m_vsync = true;
  void* m_drm_display = nullptr;

  struct gbm_device* m_gbm_device = nullptr;
  struct gbm_surface* m_fb_surface = nullptr;

  std::thread m_present_thread;
  std::mutex m_present_mutex;
  std::condition_variable m_present_cv;
  std::atomic_bool m_present_pending{false};
  std::atomic_bool m_present_thread_shutdown{false};
  std::condition_variable m_present_done_cv;
  Buffer* m_current_present_buffer = nullptr;

  u32 m_num_buffers = 0;
  std::array<Buffer, MAX_BUFFERS> m_buffers{};
};
