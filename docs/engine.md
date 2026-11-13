# Engine guide

`r-type_game-engine` is the OrByte editor. You build a scene from entities and components, save it as an `.obe` world, then compile it into a standalone game. The game ships with its world, the plugins it was compiled with, and a local server so other players can watch it.

## Start the editor

Build the project (see the [README](../README.md)), then run the editor from the repository root:

```sh
./r-type_game-engine
```

The editor loads `plugins/SFML.so` (`.dll` on Windows) for rendering and every component plugin in `plugins/components/`. A component whose plugin is missing cannot be added from the editor, and a world using it fails to load with "uses component type N which is not loaded".

## Editor layout

| Area | What it does |
|---|---|
| Toolbar | **Compile**, **Compile Options**, **Save**, **Load**, **Open Project** |
| Scenes bar | One tab per open world. Unsaved worlds are named "New Scene" |
| Hierarchy (left) | Entities of the current scene. Select one to edit it |
| Scene view (center) | Drag entities with the left button, pan with the middle button, zoom with the wheel |
| Details (right) | Components of the selected entity: edit fields, add or remove components |
| Bottom tabs | **Assets** (drag an image onto the scene), **Console**, **Compile Options** |

Every entity gets a `Name` component on creation. **Save** and **Load** work on `.obe` files in `configs/`. **Open Project** picks the folder the Assets tab browses.

## Worlds (`.obe`)

An `.obe` file is a binary dump of a world, written by `serializeWorld` and read by `deserializeWorld` (`src/GameEngine/ECS/WorldSerializer.cpp`):

| Field | Size | Meaning |
|---|---:|---|
| Magic | 4 bytes | `syat` (`WORLD_MAGIC`) |
| Entity count | 4 bytes | Number of entities |
| Per entity: id | 4 bytes | Entity id |
| Per entity: component count | 4 bytes | Number of components that follow |
| Per component: type | 1 byte | `ComponentType` value |
| Per component: data | variable | The component's own `serialize()` output |

Components carry no size field: the reader must know the exact layout of every component to stay aligned. Two consequences:

- Changing a component's `serialize()` breaks every saved world that uses it. Re-save the affected worlds from the editor after the change.
- `ComponentType` values are stored as numbers. Add new types at the end of the enum, never reorder it.

When a world fails to load, the error names the reason: "is not an OrByte world" (wrong magic), "has an invalid entity header", or "is truncated or corrupted" (usually a component layout change).

## Compile a game

1. Open **Compile Options**. Set the binary name, untick the systems and components the game does not need, and pick the features to compile in.
2. Press **Save** in that tab. The options are written to `configs/compile.options` (ignored by git).
3. Press **Compile** in the toolbar, or type `compile` in the console.

The engine exports the current scene to `/tmp/game.obe` (the temp directory on Windows), then runs CMake on `cmake/GameEngine` in `build-solo/` (`build-solo-windows/` on Windows). The output shows in the console and ends with "Compilation succeeded" or "Compilation failed". The first compile downloads the dependencies and takes a few minutes.

| Output | Linux | Windows |
|---|---|---|
| Game binary | `<name>` at the repository root | `<name>.exe` at the repository root |
| Plugins | `plugins/` (shared with the editor) | `<name>_plugins/` next to the game |

Windows cannot rewrite a DLL the running editor has loaded, so a Windows game gets its own plugin folder.

### Compile options

| Option | Effect |
|---|---|
| Binary name | Letters, digits, `_`, `-` and `.`. It cannot be one of the project binaries |
| Systems | Only the ticked systems are built and loaded. Without `Display` the game draws nothing |
| Components | Only the ticked components are active. Systems and Lua scripts see the others as missing (`getActiveComponent` returns `nullptr`, Lua gets `nil`) |
| Features | `Chat` and `Lagometer`. Each one defines `ORBYTE_FEATURE_<NAME>` when compiling the game |

Disabled components stay in the world data: the serializer still reads them so the rest of the file stays aligned.

### Console

| Command | Effect |
|---|---|
| `compile` | Export the world and build the game with the saved options |
| `cd <dir>` | Change the console directory. `cd` alone goes back to the project root |
| `clear` | Empty the console |
| `help` | List the commands |

Anything else runs in `sh` (`cmd` on Windows), for example `ls` or `git status`. One command runs at a time.

## The compiled game

The world is embedded in the binary at build time (`cmake/GameEngine/EmbedWorld.cmake` generates `GameWorld.cpp`), so the game does not need the `.obe` file to run and two games never share one. The list of compiled systems and components is embedded too (`GamePlugins.cpp`).

Run the game from the repository root, like every other binary: sprites, sounds, fonts and Lua scripts are read from relative paths (`assets/`, `scripts/`).

The main menu has three entries:

- **Play** starts the game locally with the player name entered in the field.
- **Browse server** connects to another running game and lists its rooms, to spectate them.
- **Quit** closes the window.

Every running game also hosts a server on TCP `6969` and UDP `6767`. The simulation (`server::GameInstance`) runs on its own thread at 60 ticks per second and sends snapshots to spectators. With the `Chat` feature, players and spectators share a chat (Enter to open it, Enter to send, Escape to cancel). With `Lagometer`, spectators see their ping in the top right corner.

### Tick order

The game loads every compiled system from the plugin folder and updates them in this order: `Control`, `Script`, `Physic`, `Collider`, `Health`, `Audio`, then any other system. `Display` runs separately, on the render thread. After `Collider`, the collision pairs are passed to the `Script` system so scripts receive `onCollisionEnter`.

### Audio

The `Audio` system plays `Music` components through the graphic plugin's music stream, one track at a time: the first entity whose `Music` is `Playing` holds the stream until it stops, pauses, or loses the component. A non-looping track that reaches its end switches to `Stopped`. Pausing stops the stream, and playing again restarts the track from the beginning. The `pitch` field is not applied.

## Where the code lives

| Part | Location |
|---|---|
| Editor window and layout | `src/GameEngine/GameEngine.cpp`, `src/GameEngine/Layout/` |
| Console and compile options | `src/GameEngine/Console/` |
| ECS (`World`, managers, serializer, scenes) | `src/GameEngine/ECS/`, `include/GameEngine/ECS/` |
| Component plugins | `include/GameEngine/ECS/Components/`, `src/GameEngine/Components/` |
| System plugins | `include/GameEngine/Systems/`, `src/GameEngine/Systems/` |
| Graphic plugin | `include/plugins/SFML.hpp`, `src/plugins/SFML.cpp` |
| Plugin interfaces | `include/plugins/` |
| Compiled game (client, local server) | `src/GameEngine/client/`, `src/GameEngine/server/` |
| Game build | `cmake/GameEngine/` |

To add components, systems, features or a graphic library, see the [developer guide](development.md). To script entities in Lua, see the [scripting guide](scripting.md).
