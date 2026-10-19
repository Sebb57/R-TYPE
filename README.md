# OrByte

OrByte is a C++23 multiplayer horizontal shoot 'em up. The server owns the simulation and room lifecycle; clients render snapshots and send player input.

## Build

Requirements: CMake 3.23+, a C++23 compiler, and network access on the first configure (CMake downloads Asio, bitsery, and SFML through FetchContent).

```sh
cmake -S . -B build
cmake --build build --parallel
```

Run both programs from the repository root so the relative paths to `assets/` resolve:

```sh
./build/bin/r-type_server
./build/bin/r-type_client 127.0.0.1 Player1
```

The server listens on TCP port `6969` and UDP port `6767`. Start another client to play together. The client takes an optional IPv4 address and player name; defaults are `127.0.0.1` and `Player`.

## Play

Use the arrow keys to move, Space to fire, Enter to activate a menu item, and Escape to go back or leave. In a room, every player must mark ready and complete UDP linking before the host can start. A game lasts up to three minutes; the end screen shows each player's score.

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
