# OrByte

OrByte is a C++23 multiplayer horizontal shoot 'em up. The server owns the simulation and room lifecycle; clients render snapshots and send player input.

## Build

Requirements: CMake 3.23+, a C++23 compiler, and network access on the first configure (CMake downloads Asio, bitsery, and SFML through FetchContent). The executables are written to the repository root.

### Linux

SFML needs the usual system development packages (X11/Xrandr/Xcursor, OpenGL, udev, FreeType, OpenAL, FLAC, Vorbis).

```sh
cmake -S . -B build
cmake --build build --parallel
```

### Windows (Visual Studio 2022)

From a Developer PowerShell, or open the folder directly in Visual Studio:

```powershell
cmake -S . -B build
cmake --build build --config Release --parallel
```

This produces `r-type_server.exe`, `r-type_client.exe`, and `openal32.dll` (needed by the client). SFML is linked statically, so no other DLL is needed.

### Windows (cross-compiled from Linux)

Use a GCC-based MinGW-w64 toolchain (`mingw-w64-gcc` on Arch, `g++-mingw-w64-x86-64-posix` on Debian/Ubuntu). llvm-mingw does not work: its libc++ rejects SFML 2.6.

```sh
cmake -S . -B build-windows -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake
cmake --build build-windows --parallel
```

Or without installing the toolchain, through Docker:

```sh
docker build -t rtype-windows-cross -f docker/windows-cross.Dockerfile docker
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/src rtype-windows-cross
```

To ship the Windows build, copy `r-type_server.exe`, `r-type_client.exe`, `openal32.dll`, and the `assets/` folder.

## Run

Run both programs from the repository root so the relative paths to `assets/` resolve:

```sh
./r-type_server
./r-type_client 127.0.0.1 Player1
```

On Windows, run `r-type_server.exe` and `r-type_client.exe` the same way.

The server listens on TCP port `6969` and UDP port `6767`. Start another client to play together. The client takes an optional IPv4 address and player name; defaults are `127.0.0.1` and `Player`.

## Play

Use the arrow keys to move, Space to fire, Enter to activate a menu item, and Escape to go back or leave. In a room, every player must mark ready and complete UDP linking before the host can start. A game lasts up to three minutes; the end screen shows each player's score.

## Engine component selection

Engine systems use `world.getActiveComponent<T>(entity)` to respect the component
list selected when compiling a game. It returns `nullptr` for disabled types,
even when their data is still stored on an entity. Raw `getComponent` remains
available to the editor and serializer. Without a compilation filter, all types
are active.

Lua component access follows the same filter and returns `nil` for disabled
types. Controls do not create a disabled `Velocity`. Loading another `.obe`
preserves the filter; native readers consume missing libraries' component data
so the remaining scene data stays aligned.

## Lint

The checks live in `.clang-tidy`. They read the compile database that CMake writes to `build/compile_commands.json`, so configure the project first:

```sh
cmake -S . -B build
```

```sh
run-clang-tidy -p build -quiet "$PWD/src/"              # whole project
clang-tidy -p build src/client/scenes/RoomScene.cpp     # a single file
run-clang-tidy -p build -quiet -fix "$PWD/src/"         # apply automatic fixes
```

The `"$PWD/src/"` filter keeps the run to project sources and skips the FetchContent dependencies.

## Documentation

- [Protocol reference](docs/protocol.md): TCP lobby messages and UDP gameplay packets.
- [Developer guide](docs/development.md): where to extend gameplay, entities, and the ECS engine.
- [Player guide](docs/user-guide.md): build, launch, join a room, and play.

## Project layout

- `src/server/`, `include/server/`: TCP/UDP server, sessions, rooms, and authoritative game simulation.
- `src/client/`, `include/client/`: menus, network client, and rendering scene.
- `src/Orbyte/`, `include/Orbyte/`: game entity behavior and declarations.
- `src/GameEngine/`, `include/GameEngine/`: reusable ECS and systems.
- `include/protocol/`: packet types and serialization helpers.
- `assets/`: sprites, sounds, and fonts.
