FROM ubuntu:24.04 AS builder

# Keep the vcpkg tool and dependency registry on the same pinned revision.
ARG VCPKG_REF=b1e15efef6758eaa0beb0a8732cfa66f6a68a81d
ARG BUILD_JOBS=2

ENV VCPKG_ROOT=/opt/vcpkg \
    VCPKG_DISABLE_METRICS=1 \
    VCPKG_MAX_CONCURRENCY=${BUILD_JOBS}

# Qt needs these Linux development packages even for a backend-only build.
# vcpkg installs the project's Qt, MongoDB, libharu and fmt dependencies.
# libsystemd's Meson build also requires Jinja2 in the system Python.
RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        autoconf \
        autoconf-archive \
        automake \
        bison \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        flex \
        git \
        gperf \
        libdbus-1-dev \
        libegl1-mesa-dev \
        libfontconfig1-dev \
        libglu1-mesa-dev \
        libltdl-dev \
        libsecret-1-dev \
        libtool \
        libtool-bin \
        libx11-xcb-dev \
        '^libxcb.*-dev' \
        libxi-dev \
        libxkbcommon-dev \
        libxkbcommon-x11-dev \
        libxrender-dev \
        nasm \
        ninja-build \
        perl \
        pkg-config \
        python3 \
        python3-jinja2 \
        tar \
        unzip \
        xz-utils \
        zip \
    && rm -rf /var/lib/apt/lists/*

RUN git init "${VCPKG_ROOT}" \
    && git -C "${VCPKG_ROOT}" remote add origin https://github.com/microsoft/vcpkg.git \
    && git -C "${VCPKG_ROOT}" fetch --depth 1 origin "${VCPKG_REF}" \
    && git -C "${VCPKG_ROOT}" checkout --detach FETCH_HEAD \
    && "${VCPKG_ROOT}/bootstrap-vcpkg.sh" -disableMetrics

WORKDIR /app

# Local Docker builds can reuse this layer when only application code changes.
# Heroku may rebuild it; compiling Qt from source takes time and disk space.
COPY vcpkg.json vcpkg-configuration.json ./
RUN "${VCPKG_ROOT}/vcpkg" install \
      --triplet x64-linux \
      --x-install-root=/opt/vcpkg_installed \
      --clean-after-build

COPY . .
ARG APP_VERSION=""
# Use the installed executable explicitly so project files cannot affect lookup.
RUN /usr/bin/cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      ${APP_VERSION:+-DINVOICE_APP_VERSION=${APP_VERSION}} \
      -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=x64-linux \
      -DVCPKG_INSTALLED_DIR=/opt/vcpkg_installed \
    && /usr/bin/cmake --build build --target InvokeInvoiceBackend --parallel "${BUILD_JOBS}"

FROM builder AS tests
RUN /usr/bin/cmake --build build --target all-tests --parallel "${BUILD_JOBS}"
CMD ["/usr/bin/ctest", "--test-dir", "/app/build", "--output-on-failure", "--no-tests=error"]

FROM builder AS runtime
# Retain build tools and installed libraries in this initial image.
# The application itself does not need root privileges.
RUN useradd --create-home --uid 10001 appuser \
    && chown appuser:appuser /app
USER appuser

CMD ["/app/build/InvokeInvoiceBackend"]
