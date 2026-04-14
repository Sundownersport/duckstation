// SPDX-FileCopyrightText: 2021-2026 Connor McLaughlin <stenzek@gmail.com>
// SPDX-License-Identifier: CC-BY-NC-ND-4.0
// Ported from the original DuckStation DRM/GBM support, adapted to current EGL context API.

#include "opengl_context_egl_gbm.h"

#include "common/assert.h"
#include "common/error.h"
#include "common/log.h"

#include <cerrno>
#include <drm.h>
#include <drm_fourcc.h>
#include <gbm.h>

LOG_CHANNEL(GPUDevice);

OpenGLContextEGLGBM::OpenGLContextEGLGBM()
{
  StartPresentThread();
}

OpenGLContextEGLGBM::~OpenGLContextEGLGBM()
{
  StopPresentThread();
  Assert(!m_current_present_buffer);

  while (m_num_buffers > 0)
  {
    Buffer& buffer = m_buffers[--m_num_buffers];
    GetDRM()->RemoveBuffer(buffer.fb_id);
  }

  if (m_fb_surface)
    gbm_surface_destroy(m_fb_surface);

  if (m_gbm_device)
    gbm_device_destroy(m_gbm_device);
}

std::unique_ptr<OpenGLContext> OpenGLContextEGLGBM::Create(WindowInfo& wi, SurfaceHandle* surface,
                                                           std::span<const Version> versions_to_try, Error* error)
{
  std::unique_ptr<OpenGLContextEGLGBM> context = std::make_unique<OpenGLContextEGLGBM>();
  context->m_drm_display = wi.display_connection;

  if (!context->CreateGBMDevice())
  {
    Error::SetStringView(error, "Failed to create GBM device");
    return nullptr;
  }

  if (!context->Initialize(wi, surface, versions_to_try, error))
    return nullptr;

  return context;
}

std::unique_ptr<OpenGLContext> OpenGLContextEGLGBM::CreateSharedContext(WindowInfo& wi, SurfaceHandle* surface,
                                                                        Error* error)
{
  std::unique_ptr<OpenGLContextEGLGBM> context = std::make_unique<OpenGLContextEGLGBM>();
  context->m_drm_display = m_drm_display;
  context->m_display = m_display;

  if (!context->CreateContextAndSurface(wi, surface, m_version, m_context, false, error))
    return nullptr;

  return context;
}

void OpenGLContextEGLGBM::ResizeSurface(WindowInfo& wi, SurfaceHandle handle)
{
  OpenGLContextEGL::ResizeSurface(wi, handle);
}

bool OpenGLContextEGLGBM::CreateGBMDevice()
{
  Assert(!m_gbm_device);
  m_gbm_device = gbm_create_device(GetDRM()->GetCardFD());
  if (!m_gbm_device)
  {
    ERROR_LOG("gbm_create_device() failed: {}", errno);
    return false;
  }

  return true;
}

EGLDisplay OpenGLContextEGLGBM::GetPlatformDisplay(const WindowInfo& wi, Error* error)
{
  EGLDisplay dpy = TryGetPlatformDisplay(m_gbm_device, EGL_PLATFORM_GBM_KHR, "EGL_KHR_platform_gbm");
  if (dpy == EGL_NO_DISPLAY)
    dpy = GetFallbackDisplay(m_gbm_device, error);
  return dpy;
}

EGLSurface OpenGLContextEGLGBM::CreatePlatformSurface(EGLConfig config, const WindowInfo& wi, Error* error)
{
  EGLint visual_id;
  eglGetConfigAttrib(m_display, config, EGL_NATIVE_VISUAL_ID, &visual_id);

  if (m_fb_surface)
  {
    gbm_surface_destroy(m_fb_surface);
    m_fb_surface = nullptr;
  }

  m_fb_surface = gbm_surface_create(m_gbm_device, GetDRM()->GetWidth(), GetDRM()->GetHeight(),
                                    static_cast<u32>(visual_id), GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT);
  if (!m_fb_surface)
  {
    Error::SetStringFmt(error, "gbm_surface_create() failed: {}", errno);
    return EGL_NO_SURFACE;
  }

  EGLSurface egl_surface = TryCreatePlatformSurface(config, m_fb_surface, error);
  if (egl_surface == EGL_NO_SURFACE)
    egl_surface = CreateFallbackSurface(config, m_fb_surface, error);

  return egl_surface;
}

OpenGLContextEGLGBM::Buffer* OpenGLContextEGLGBM::LockFrontBuffer()
{
  struct gbm_bo* bo = gbm_surface_lock_front_buffer(m_fb_surface);
  if (!bo)
    return nullptr;

  Buffer* buffer = nullptr;
  for (u32 i = 0; i < m_num_buffers; i++)
  {
    if (m_buffers[i].bo == bo)
    {
      buffer = &m_buffers[i];
      break;
    }
  }

  if (!buffer)
  {
    Assert(m_num_buffers < MAX_BUFFERS);

    const u32 width = gbm_bo_get_width(bo);
    const u32 height = gbm_bo_get_height(bo);
    const u32 stride = gbm_bo_get_stride(bo);
    const u32 format = gbm_bo_get_format(bo);
    const u32 handle = gbm_bo_get_handle(bo).u32;

    std::optional<u32> fb_id = GetDRM()->AddBuffer(width, height, format, handle, stride, 0);
    if (!fb_id.has_value())
      return nullptr;

    buffer = &m_buffers[m_num_buffers];
    buffer->bo = bo;
    buffer->fb_id = fb_id.value();
    m_num_buffers++;
  }

  return buffer;
}

void OpenGLContextEGLGBM::ReleaseBuffer(Buffer* buffer)
{
  gbm_surface_release_buffer(m_fb_surface, buffer->bo);
}

void OpenGLContextEGLGBM::PresentBuffer(Buffer* buffer, bool wait_for_vsync)
{
  GetDRM()->PresentBuffer(buffer->fb_id, wait_for_vsync);
}

bool OpenGLContextEGLGBM::SwapBuffers()
{
  if (!OpenGLContextEGL::SwapBuffers())
    return false;

  std::unique_lock lock(m_present_mutex);
  m_present_pending.store(true);
  m_present_cv.notify_one();
  if (m_vsync)
    m_present_done_cv.wait(lock, [this]() { return !m_present_pending.load(); });

  return true;
}

bool OpenGLContextEGLGBM::SetSwapInterval(s32 interval, Error* error)
{
  if (interval < 0 || interval > 1)
  {
    Error::SetStringView(error, "Only swap intervals 0 and 1 are supported with DRM/GBM");
    return false;
  }

  std::unique_lock lock(m_present_mutex);
  m_vsync = (interval > 0);
  return true;
}

void OpenGLContextEGLGBM::StartPresentThread()
{
  m_present_thread_shutdown.store(false);
  m_present_thread = std::thread(&OpenGLContextEGLGBM::PresentThread, this);
}

void OpenGLContextEGLGBM::StopPresentThread()
{
  if (!m_present_thread.joinable())
    return;

  {
    std::unique_lock lock(m_present_mutex);
    m_present_thread_shutdown.store(true);
    m_present_cv.notify_one();
  }

  m_present_thread.join();
}

void OpenGLContextEGLGBM::PresentThread()
{
  std::unique_lock lock(m_present_mutex);

  while (!m_present_thread_shutdown.load())
  {
    m_present_cv.wait(lock);

    if (!m_present_pending.load())
      continue;

    Buffer* next_buffer = LockFrontBuffer();
    const bool wait_for_vsync = m_vsync && m_current_present_buffer;

    lock.unlock();
    if (next_buffer)
      PresentBuffer(next_buffer, wait_for_vsync);
    lock.lock();

    if (m_current_present_buffer)
      ReleaseBuffer(m_current_present_buffer);

    m_current_present_buffer = next_buffer;
    m_present_pending.store(false);
    m_present_done_cv.notify_one();
  }

  if (m_current_present_buffer)
  {
    ReleaseBuffer(m_current_present_buffer);
    m_current_present_buffer = nullptr;
  }
}
