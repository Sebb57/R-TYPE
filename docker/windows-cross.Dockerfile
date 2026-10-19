FROM debian:trixie-slim

RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates cmake git ninja-build g++-mingw-w64-x86-64-posix && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix && update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix

WORKDIR /src
ENV HOME=/tmp
CMD ["sh", "-c", "cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake && cmake --build build-windows --parallel"]
