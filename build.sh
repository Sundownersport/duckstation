#!/bin/bash
set -e

# ccache setup
export CCACHE_DIR="${CCACHE_DIR:-/ccache}"
export PATH="/usr/lib/ccache:$PATH"
ccache --max-size=1G
ccache --zero-stats

# Create toolchain file for aarch64 cross-compilation
cat > /tmp/toolchain.cmake << 'EOF'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER clang-19)
set(CMAKE_CXX_COMPILER clang++-19)
set(CMAKE_C_COMPILER_TARGET aarch64-linux-gnu)
set(CMAKE_CXX_COMPILER_TARGET aarch64-linux-gnu)
set(CMAKE_C_COMPILER_AR llvm-ar-19)
set(CMAKE_C_COMPILER_RANLIB llvm-ranlib-19)
set(CMAKE_CXX_COMPILER_AR llvm-ar-19)
set(CMAKE_CXX_COMPILER_RANLIB llvm-ranlib-19)
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld -L/usr/lib/gcc-cross/aarch64-linux-gnu/9")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld -L/usr/lib/gcc-cross/aarch64-linux-gnu/9")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld -L/usr/lib/gcc-cross/aarch64-linux-gnu/9")
set(CMAKE_SYSROOT /usr/aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH "/src/dep/prebuilt/linux-cross-arm64;/usr/aarch64-linux-gnu;/usr/lib/aarch64-linux-gnu")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(PKG_CONFIG_EXECUTABLE /usr/bin/aarch64-linux-gnu-pkg-config)
EOF

cd /src

cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
  -DCMAKE_TOOLCHAIN_FILE=/tmp/toolchain.cmake \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DBUILD_SDL_FRONTEND=ON \
  -DENABLE_X11=OFF \
  -DENABLE_WAYLAND=OFF \
  -DENABLE_GBM=ON \
  -DHOST_MIN_PAGE_SIZE=4096 \
  -DHOST_MAX_PAGE_SIZE=16384 \
  -DHOST_CACHE_LINE_SIZE=64

cmake --build build --parallel

# Package
OUTPUT_DIR="/output"
OUTDIR="$OUTPUT_DIR/duckstation-aarch64"
mkdir -p "$OUTDIR/lib"

cp build/bin/duckstation-sdl "$OUTDIR/"
if [ -d build/bin/resources ]; then
  cp -r build/bin/resources "$OUTDIR/"
fi

# Collect shared library dependencies
CROSS=aarch64-linux-gnu
DEPS="/src/dep/prebuilt/linux-cross-arm64"
SKIP_LIBS="linux-vdso|ld-linux|libc\.so|libm\.so|libdl\.so|libpthread\.so|librt\.so|libgcc_s|libstdc\+\+|libSDL2|libdrm|libgbm|libEGL|libGLES|libMali|libudev|libasound"

collect_lib() {
  local lib="$1"
  [ -f "$OUTDIR/lib/$lib" ] && return
  echo "$lib" | grep -qE "$SKIP_LIBS" && return
  local src
  src=$(find "$DEPS/lib" "/usr/lib/$CROSS" "/lib/$CROSS" -maxdepth 2 -name "$lib" 2>/dev/null | head -1)
  if [ -n "$src" ]; then
    cp -L "$src" "$OUTDIR/lib/$lib"
    echo "  Collected: $lib"
  fi
}

echo "=== Collecting dependencies ==="
${CROSS}-readelf -d "$OUTDIR/duckstation-sdl" 2>/dev/null | grep NEEDED | \
  sed 's/.*\[\(.*\)\]/\1/' | while read -r lib; do collect_lib "$lib"; done

for so in "$OUTDIR"/lib/*.so*; do
  [ -f "$so" ] || continue
  ${CROSS}-readelf -d "$so" 2>/dev/null | grep NEEDED | \
    sed 's/.*\[\(.*\)\]/\1/' | while read -r lib; do collect_lib "$lib"; done
done

# Set RPATH and strip
patchelf --set-rpath '$ORIGIN/lib' "$OUTDIR/duckstation-sdl"
for so in "$OUTDIR"/lib/*.so*; do
  [ -f "$so" ] || continue
  patchelf --set-rpath '$ORIGIN' "$so" 2>/dev/null || true
done

llvm-strip "$OUTDIR/duckstation-sdl" 2>/dev/null || true
for so in "$OUTDIR"/lib/*.so*; do
  [ -f "$so" ] || continue
  llvm-strip "$so" 2>/dev/null || true
done

cd "$OUTPUT_DIR"
tar -czf duckstation-aarch64.tar.gz duckstation-aarch64/

echo "=== Package contents ==="
ls -la "$OUTDIR/"
ls -la "$OUTDIR/lib/" 2>/dev/null || true
ls -la "$OUTPUT_DIR/duckstation-aarch64.tar.gz"

echo "=== ccache stats ==="
ccache --show-stats
