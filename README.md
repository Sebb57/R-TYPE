# OrByte

OrByte is a C++23 multiplayer horizontal shoot 'em up and the engine it is built on. The server owns the simulation and room lifecycle; clients render snapshots and send player input. The engine is an editor that builds scenes from ECS components and compiles them into standalone games, scriptable in Lua.

## Build

Requirements: CMake 3.28+, a C++23 compiler, and network access on the first configure (CMake downloads Asio, bitsery, SFML 3, Lua, and sol2 through FetchContent). The executables are written to the repository root, the plugins to `plugins/`.

### Linux

SFML needs the X11 (with Xrandr, Xcursor and Xi), OpenGL and udev development packages. FreeType, FLAC and Vorbis are downloaded with SFML.

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

This produces `r-type_server.exe`, `r-type_client.exe`, `r-type_game-engine.exe`, and the plugin DLLs in `plugins/`. SFML is linked statically, so no other DLL is needed.

### Windows (cross-compiled from Linux)

Use a GCC-based MinGW-w64 toolchain (`mingw-w64-gcc` on Arch, `g++-mingw-w64-x86-64-posix` on Debian/Ubuntu). The Docker image below uses the posix variant; llvm-mingw is not tested.

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

To ship the Windows build, copy the `.exe` files, the `plugins/` folder, and the `assets/` folder.

## Run

Run the programs from the repository root so the relative paths to `assets/`, `plugins/`, and `scripts/` resolve:

```sh
./r-type_server
./r-type_client 127.0.0.1 Player1
./r-type_game-engine
```

On Windows, run the `.exe` files the same way.

The server listens on TCP port `6969` and UDP port `6767`. Start another client to play together. The client takes an optional IPv4 address and player name; defaults are `127.0.0.1` and `Player`.

## Play

Use the arrow keys to move, Space to fire, Enter to activate a menu item, and Escape to go back or leave. In a room, every player must mark ready and complete UDP linking before the host can start. A game lasts up to three minutes; the end screen shows each player's score.

## Engine

Build a scene in the editor, then press **Compile**: the engine builds a standalone game from the current scene, with the world embedded in the binary, written to the repository root. Pick its name, systems, components, and features (`Chat`, `Lagometer`) in the **Compile Options** tab first. The [engine guide](docs/engine.md) covers the editor, the `.obe` world format, and the compiled game.

Entities are scripted in Lua through the `Script` component: see the [scripting guide](docs/scripting.md) and the Starfall example in `scripts/starfall/` (world `configs/starfall.obe`).

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

- [Engine guide](docs/engine.md): the editor, `.obe` worlds, compiling a game, and how the compiled game runs.
- [Scripting guide](docs/scripting.md): Lua callbacks, API, and component fields.
- [Developer guide](docs/development.md): adding components, systems, compile features, graphic libraries, and R-Type gameplay.
- [Protocol reference](docs/protocol.md): TCP lobby messages and UDP gameplay packets.
- [Player guide](docs/user-guide.md): build, launch, join a room, and play.

## Project layout

- `src/server/`, `include/server/`: TCP/UDP server, sessions, rooms, and authoritative game simulation.
- `src/client/`, `include/client/`: menus, network client, and rendering scene.
- `src/Orbyte/`, `include/Orbyte/`: game entity behavior and declarations.
- `src/GameEngine/`, `include/GameEngine/`: ECS, component and system plugins, the editor, and the compiled game (`client/`, `server/`).
- `src/plugins/`, `include/plugins/`: the SFML graphic plugin and the plugin interfaces.
- `cmake/GameEngine/`: the CMake project the editor runs to compile a game.
- `include/protocol/`: packet types and serialization helpers.
- `configs/`: saved worlds (`.obe`) and the compile options.
- `scripts/`: Lua scripts.
- `assets/`: sprites, sounds, and fonts.
