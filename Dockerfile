FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Configure multiarch for arm64 cross-compilation (glibc 2.31)
RUN dpkg --add-architecture arm64 && \
    sed -i 's/^deb http/deb [arch=amd64] http/g' /etc/apt/sources.list && \
    echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports focal main restricted universe multiverse" >> /etc/apt/sources.list && \
    echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports focal-updates main restricted universe multiverse" >> /etc/apt/sources.list

# Add LLVM and Kitware apt repositories (keys first, then repos)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    wget gnupg software-properties-common apt-transport-https ca-certificates && \
    # Kitware (CMake) - add key BEFORE repository
    wget -qO- https://apt.kitware.com/keys/kitware-archive-latest.asc | apt-key add - && \
    echo "deb [arch=amd64] https://apt.kitware.com/ubuntu/ focal main" >> /etc/apt/sources.list && \
    # LLVM (Clang 19)
    wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    echo "deb [arch=amd64] http://apt.llvm.org/focal/ llvm-toolchain-focal-19 main" >> /etc/apt/sources.list

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    # Build tools
    build-essential cmake ninja-build git pkg-config ccache patchelf python3 \
    # Clang 19 cross-compilation toolchain
    clang-19 lld-19 llvm-19 \
    # ARM64 cross-compiler (needed for some system libs)
    gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
    # ARM64 development libraries
    libsdl2-dev:arm64 \
    libdrm-dev:arm64 \
    libgbm-dev:arm64 \
    libegl1-mesa-dev:arm64 \
    libgles2-mesa-dev:arm64 \
    libasound2-dev:arm64 \
    libpulse-dev:arm64 \
    libcurl4-openssl-dev:arm64 \
    libudev-dev:arm64 \
    zlib1g-dev:arm64 \
    libglib2.0-dev:arm64 \
    && rm -rf /var/lib/apt/lists/*

# Create symlinks for tools
RUN ln -sf /usr/bin/clang-19 /usr/bin/clang && \
    ln -sf /usr/bin/clang++-19 /usr/bin/clang++ && \
    ln -sf /usr/bin/lld-19 /usr/bin/lld && \
    ln -sf /usr/bin/ld.lld-19 /usr/bin/ld.lld && \
    ln -sf /usr/bin/llvm-ar-19 /usr/bin/llvm-ar && \
    ln -sf /usr/bin/llvm-ranlib-19 /usr/bin/llvm-ranlib && \
    ln -sf /usr/bin/llvm-strip-19 /usr/bin/llvm-strip && \
    ln -sf /usr/bin/llvm-objcopy-19 /usr/bin/llvm-objcopy

WORKDIR /build
