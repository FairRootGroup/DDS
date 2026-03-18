# syntax=docker/dockerfile:1
#
# Multi-stage dev container for the DDS C++ project.
#
# Stage 1 (builder) compiles all third-party C++ dependencies from source
# so their build tools and source trees never appear in the final image.
#
# Stage 2 (dev) is the image you actually run:
#   - debian:bookworm-slim base (~30 MB)
#   - Clang/LLVM 20 (compiler, clangd, clang-format, lld)
#   - CMake 4.2.3 (pre-built binary, no compilation needed)
#   - Boost 1.90.0 shared libs + headers (built with GCC in stage 1)
#   - Protobuf v34.0 + abseil (built in stage 1)
#   - ccache + git + make
#
# Supports linux/amd64 and linux/arm64 via Docker TARGETARCH.

# ----------------------------------------------------------------
# Stage 1: Builder — compile all C++ dependencies from source
# ----------------------------------------------------------------
FROM debian:bookworm-slim AS builder

ARG TARGETARCH
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        pkg-config \
        python3 \
        wget \
    && rm -rf /var/lib/apt/lists/*

# ── CMake 4.2.3 (pre-built binary, architecture-aware) ──────────────────────
RUN set -eux; \
    ARCH=$([ "$TARGETARCH" = "arm64" ] && echo "aarch64" || echo "x86_64"); \
    wget -q "https://github.com/Kitware/CMake/releases/download/v4.2.3/cmake-4.2.3-linux-${ARCH}.tar.gz" \
         -O /tmp/cmake.tar.gz; \
    tar -xzf /tmp/cmake.tar.gz -C /opt; \
    mv /opt/cmake-4.2.3-linux-${ARCH} /opt/cmake; \
    rm /tmp/cmake.tar.gz

ENV PATH="/opt/cmake/bin:$PATH"

# ── Boost 1.90.0 (only required components; all headers always installed) ────
#    Built without ICU (matches docs/3rd-party.md recommendation).
#    link=shared → smaller install; variant=release → optimised libs.
RUN set -eux; \
    wget -q "https://archives.boost.io/release/1.90.0/source/boost_1_90_0.tar.gz" \
         -O /tmp/boost.tar.gz; \
    tar -xzf /tmp/boost.tar.gz -C /tmp; \
    cd /tmp/boost_1_90_0; \
    ./bootstrap.sh --prefix=/opt/deps --without-icu; \
    ./b2 install \
        --prefix=/opt/deps \
        --with-log \
        --with-thread \
        --with-program_options \
        --with-filesystem \
        --with-regex \
        --with-system \
        --with-test \
        --layout=system \
        threading=multi \
        link=shared \
        variant=release \
        cxxstd=17 \
        -j"$(nproc)"; \
    rm -rf /tmp/boost_1_90_0 /tmp/boost.tar.gz

# ── abseil-cpp 20240722.1 (required by Protobuf v34) ─────────────────────────
RUN set -eux; \
    wget -q "https://github.com/abseil/abseil-cpp/archive/refs/tags/20240722.1.tar.gz" \
         -O /tmp/abseil.tar.gz; \
    tar -xzf /tmp/abseil.tar.gz -C /tmp; \
    cmake -S /tmp/abseil-cpp-20240722.1 -B /tmp/abseil-build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/deps \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -DABSL_PROPAGATE_CXX_STD=ON \
        -DCMAKE_CXX_STANDARD=17 \
        -DABSL_ENABLE_INSTALL=ON; \
    cmake --build /tmp/abseil-build -j"$(nproc)"; \
    cmake --install /tmp/abseil-build; \
    rm -rf /tmp/abseil-cpp-20240722.1 /tmp/abseil-build /tmp/abseil.tar.gz

# ── Protobuf v34.0 (uses pre-built abseil via -DCMAKE_PREFIX_PATH) ───────────
RUN set -eux; \
    wget -q "https://github.com/protocolbuffers/protobuf/releases/download/v34.0/protobuf-34.0.tar.gz" \
         -O /tmp/protobuf.tar.gz; \
    tar -xzf /tmp/protobuf.tar.gz -C /tmp; \
    cmake -S /tmp/protobuf-34.0 -B /tmp/protobuf-build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/deps \
        -DCMAKE_PREFIX_PATH=/opt/deps \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_EXAMPLES=OFF \
        -Dprotobuf_ABSL_PROVIDER=package \
        -DCMAKE_CXX_STANDARD=17 \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON; \
    cmake --build /tmp/protobuf-build -j"$(nproc)"; \
    cmake --install /tmp/protobuf-build; \
    rm -rf /tmp/protobuf-34.0 /tmp/protobuf-build /tmp/protobuf.tar.gz


# ----------------------------------------------------------------
# Stage 2: Dev — minimal image with Clang 20 + pre-built deps
# ----------------------------------------------------------------
FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

# Add the LLVM 20 apt repository and install Clang toolchain + dev tools.
# Consolidating into a single RUN minimises image layers.
# wget/gnupg are only needed to bootstrap the apt key — purged afterwards.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        gnupg \
        wget \
    && wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
        | gpg --dearmor -o /usr/share/keyrings/llvm-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/llvm-archive-keyring.gpg] \
        http://apt.llvm.org/bookworm/ llvm-toolchain-bookworm-21 main" \
        > /etc/apt/sources.list.d/llvm-20.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
        clang-21 \
        clangd-21 \
        clang-format-21 \
        lld-21 \
        libstdc++-12-dev \
        ccache \
        git \
        make \
        openssh-client \
        libxml2-utils \
    && apt-get purge -y wget gnupg \
    && apt-get autoremove -y \
    && rm -rf /var/lib/apt/lists/* \
    # Generate en_US.UTF-8 locale (needed by Boost.Process / std::locale)
    && apt-get update \
    && apt-get install -y --no-install-recommends locales \
    && rm -rf /var/lib/apt/lists/* \
    && sed -i '/en_US.UTF-8/s/^# //' /etc/locale.gen \
    && locale-gen

# Version-independent symlinks
RUN ln -sf /usr/bin/clang-21        /usr/local/bin/clang        \
 && ln -sf /usr/bin/clang++-21      /usr/local/bin/clang++      \
 && ln -sf /usr/bin/clang-format-21 /usr/local/bin/clang-format \
 && ln -sf /usr/bin/clangd-21       /usr/local/bin/clangd       \
 && ln -sf /usr/bin/lld-21          /usr/local/bin/lld

# ── Copy CMake 4.2.3 from builder ─────────────────────────────────────────────
COPY --from=builder /opt/cmake /opt/cmake

# ── Copy compiled deps (Boost + abseil + Protobuf) into /usr/local ────────────
#    Merges lib/, include/, bin/, lib/cmake/ etc. — all default CMake search paths.
COPY --from=builder /opt/deps /usr/local

# Register newly copied shared libraries with the dynamic linker
RUN ldconfig

# ── Non-root user (UID 1000 matches the typical host developer account) ────────
RUN useradd -m -u 1000 -s /bin/bash dds

# Persistent ccache directory; exposed as a named Docker volume in devcontainer.json
RUN mkdir -p /ccache && chown dds:dds /ccache

ENV PATH="/opt/cmake/bin:$PATH" \
    CC=clang-21 \
    CXX=clang++-21 \
    CCACHE_DIR=/ccache \
    LANG=en_US.UTF-8

WORKDIR /workspace
USER dds
