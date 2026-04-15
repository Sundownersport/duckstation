// glibc compatibility shim for running on glibc 2.31 (Ubuntu 20.04) devices.
// Provides symbols that were moved/added in glibc 2.32-2.34.
//
// In glibc 2.34, libpthread/libdl/librt were merged into libc. The symbols
// exist in older glibc but in separate libraries (libpthread.so.0, libdl.so.2,
// librt.so.1). We redirect to the older versioned symbols.

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// ============================================================================
// __libc_single_threaded (GLIBC_2.32) — not in 2.31, provide a stub
// ============================================================================
char __libc_single_threaded __attribute__((weak)) = 0;

// ============================================================================
// strerrordesc_np / strerrorname_np (GLIBC_2.32) — not in 2.31
// ============================================================================
__attribute__((weak))
const char* strerrordesc_np(int errnum)
{
  return strerror(errnum);
}

__attribute__((weak))
const char* strerrorname_np(int errnum)
{
  // Return a generic string — the caller uses this for debug logging only
  (void)errnum;
  return "UNKNOWN";
}

// ============================================================================
// stat/fstat/lstat (GLIBC_2.33) — redirect to old versions via __xstat etc.
// On aarch64, glibc 2.17 provides stat/fstat/lstat directly.
// The 2.33 versions use the same ABI on 64-bit systems.
// We use asm labels to bind to the old GLIBC_2.17 versions.
// ============================================================================

// On aarch64 with 64-bit time_t, stat/fstat/lstat have the same ABI
// across versions. The version bump was for 32-bit time_t transition.
// Force linking to GLIBC_2.17 versions.
__asm__(".symver stat_compat, stat@GLIBC_2.17");
__asm__(".symver fstat_compat, fstat@GLIBC_2.17");
__asm__(".symver lstat_compat, lstat@GLIBC_2.17");

extern int stat_compat(const char* path, struct stat* buf);
extern int fstat_compat(int fd, struct stat* buf);
extern int lstat_compat(const char* path, struct stat* buf);

// Provide GLIBC_2.33 versions that call the 2.17 versions
int __wrap_stat(const char* path, struct stat* buf) { return stat_compat(path, buf); }
int __wrap_fstat(int fd, struct stat* buf) { return fstat_compat(fd, buf); }
int __wrap_lstat(const char* path, struct stat* buf) { return lstat_compat(path, buf); }
