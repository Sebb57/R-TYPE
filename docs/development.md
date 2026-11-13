# Developer guide

## Build and run

Use the commands in the [README](../README.md). Run every program from the repository root: assets are loaded from relative `./assets/...` paths. The server listens on TCP `6969` and UDP `6767` by default; those values are in `server::ServerConfig`.

## Where to make a change

| Goal | Main location |
|---|---|
| Add or adjust server-authoritative gameplay | `src/server/game/GameInstance.cpp`, `include/server/game/GameInstance.hpp` |
| Define game entity components and actions | `src/Orbyte/` and `include/Orbyte/` |
| Add a client visual or animation | `src/client/scenes/GameScene.cpp` or the entity's `CreateClient...` method |
| Change the room/lobby lifecycle | `src/server/lobby/`, then the matching client scene under `src/client/scenes/` |
| Change network messages | `include/protocol/`, server serializers/handlers, and `src/client/network/NetworkClient.cpp` together |
| Add a reusable ECS component or system | `include/GameEngine/ECS/Components/` or `include/GameEngine/Systems/`, plus their plugin source under `src/GameEngine/` |
| Change the editor | `src/GameEngine/Layout/`, `src/GameEngine/Console/` |
| Change the game compiled by the editor | `src/GameEngine/client/`, `src/GameEngine/server/`, `cmake/GameEngine/` |

This page covers two programs that share the ECS: the multiplayer R-Type (`r-type_server`, `r-type_client`) and the engine (`r-type_game-engine`) with the games it compiles. The [engine guide](engine.md) explains how the editor and compiled games work.

In the multiplayer R-Type, the server owns health, collisions, scoring, spawn timing, player input, and entity lifetime. Clients send held input and mirror server snapshots. Keep gameplay decisions on the server; use events for short-lived visual/audio feedback. For example, explosions are local client entities created from death events and have no collider.

## Adding an entity

1. Add the entity interface under `include/Orbyte/` and its implementation under `src/Orbyte/`.
2. Put server setup in its `CreateServeur...` method: create the collider, transform, velocity, health, and ownership components needed by the simulation.
3. Put client presentation in its `CreateClient...` method: use a Sprite or Animator and the corresponding asset. Do not add rendering-only colliders.
4. Register the type in `server::EntityType` and update `GameInstance::spawn`, snapshots, and the client `GameScene::spawn` when the network must represent it.
5. Add collision, cleanup, and out-of-bounds behavior to the server simulation where applicable.

Follow the existing entity pattern: entity methods own their component setup; the game loop decides when to construct, update, or destroy them. Keep the ECS core reusable and put game-specific policy in the game/server layer.

## Adding a gameplay mechanic

For a server-only rule, implement it in `GameInstance` or the relevant entity method. Entity behavior such as steering, firing, health changes, and bonus application already has entity methods; reuse those rather than assembling components in the game loop. If the client needs a visible effect, add an event type and send it through the existing event DTO. Decode it in `NetworkClient`, then handle presentation in `GameScene`.

## How the engine is split

Components, systems and the graphic library are plugins: shared libraries in `plugins/` (`.so` on Linux, `.dll` on Windows) that the programs load at runtime. Every plugin exports two C functions:

- `type()` returns its `DynamicLibrary::PluginType` (`COMPONENT`, `SYSTEMS` or `GRAPHICAL_LIB`).
- `factory(...)` builds the instance: a component, a system, or the `IGraphic`.

| Plugin | Built from | Output |
|---|---|---|
| Component `X` | `src/GameEngine/Components/X.cpp` | `plugins/components/X.so` |
| System `X` | `src/GameEngine/Systems/XSystem.cpp` | `plugins/systems/XSystem.so` |
| Graphic library | `src/plugins/SFML.cpp` | `plugins/SFML.so` |

The main `CMakeLists.txt` lists the plugins it builds (`COMPONENT_PLUGINS`, `SYSTEM_PLUGINS`). The game build in `cmake/GameEngine/` finds them by file name, so a new plugin is picked up there without editing it.

The `World` (`include/GameEngine/ECS/World.hpp`) stores components by entity id and `ComponentType`. Systems read them with `world.getActiveComponent<T>(entity)`, which returns `nullptr` when the entity has no such component or when the type was disabled in the compile options. Use `getComponent` only in editor or serializer code that must see every component.

## Adding a component to the Game Engine

A component is plain data attached to an entity. The example adds a `Shield` with a number of points.

1. Add `Shield` to `ComponentType` in `include/plugins/IComponent.hpp`, right before `ENUM_END`. The value is written in `.obe` files: never reorder the enum or insert in the middle.
2. Create `include/GameEngine/ECS/Components/Shield.hpp`:

   ```cpp
   #pragma once
   #ifndef SHIELD_HPP_
       #define SHIELD_HPP_

       #include "plugins/AComponent.hpp"

   namespace OrByte {

   struct Shield : public OrByte::AComponent<OrByte::ComponentType::Shield>
   {
       Shield() = default;
       Shield(float points) : points(points) {}

       float points{100.0F};

       void serialize(Serializer& serializer) const final { serializer.value4b(this->points); }
       void deserialize(Deserializer& deserializer) final { deserializer.value4b(this->points); }
   };

   } // namespace OrByte

   #endif /* SHIELD_HPP_ */
   ```

   `serialize` and `deserialize` must read exactly what they write: `value4b` for `float`/`int`, `value1b` for one-byte enums, `boolValue` for `bool`, `text1b(value, MaxStringSize)` for strings, `object` for `Vector2` and `RGBA`.
3. Create the plugin, `src/GameEngine/Components/Shield.cpp`:

   ```cpp
   #include "GameEngine/ECS/Components/Shield.hpp"
   #include "plugins/DynamicLibrary.hpp"
   #include "plugins/IComponent.hpp"

   extern "C" {
       OrByte::DynamicLibrary::PluginType type()
       {
           return OrByte::DynamicLibrary::PluginType::COMPONENT;
       }

       OrByte::IComponent *factory()
       {
           return new OrByte::Shield();
       }
   }
   ```

   The file name must match the enum value (`Shield`): the compile options and the compiled game find components by that name.
4. Add `Shield` to `COMPONENT_PLUGINS` in `CMakeLists.txt`.
5. Add `OrByte::Shield` to the `nativeReaders` list in `src/GameEngine/ECS/WorldSerializer.cpp`. Worlds stay readable when the plugin is not loaded.
6. Make it editable in `src/GameEngine/Layout/Detail.cpp`: a label in `componentName`, and its fields in `describe` (helpers: `number`, `vector2`, `color`, `text`, `texture`, `toggle`, `choice`):

   ```cpp
   case ComponentType::Shield: {
       auto& c = static_cast<Shield&>(component);
       return { number("Points", c.points) };
   }
   ```

7. Expose it to Lua in `ScriptSystem::_bindComponents` (`src/GameEngine/Systems/ScriptSystem.cpp`):

   ```cpp
   this->_bindComponent<Shield>("points", &Shield::points);
   ```

Rebuild, restart the editor, and **Shield** appears in the Details panel and the Compile Options tab.

Changing the fields of an existing component changes its `.obe` layout: load and re-save every world in `configs/` that uses it, or they fail to load with "is truncated or corrupted".

## Adding a system to the Game Engine

A system holds behavior: each tick it reads and updates the components it cares about. The example drains a `Shield` before the entity loses health.

1. Declare it in `include/GameEngine/Systems/ShieldSystem.hpp`:

   ```cpp
   #pragma once
   #ifndef SHIELDSYSTEM_HPP_
       #define SHIELDSYSTEM_HPP_

       #include "GameEngine/ECS/World.hpp"
       #include "plugins/ASystem.hpp"

   namespace OrByte {

   class ShieldSystem : public ASystem {
       World& _world;

       public:
           explicit ShieldSystem(World& world) : _world(world) {}

           void update() override;
   };

   } // namespace OrByte

   #endif /* SHIELDSYSTEM_HPP_ */
   ```

2. Implement it in `src/GameEngine/Systems/ShieldSystem.cpp`, and export the plugin functions at the end of the file:

   ```cpp
   #include "GameEngine/Systems/ShieldSystem.hpp"
   #include "GameEngine/ECS/Components/Health.hpp"
   #include "GameEngine/ECS/Components/Shield.hpp"
   #include "plugins/DynamicLibrary.hpp"
   #include "plugins/ISystem.hpp"

   void OrByte::ShieldSystem::update()
   {
       for (int entity : this->_world.getExistingEntities()) {
           auto *shield = this->_world.getActiveComponent<Shield>(entity);
           auto *health = this->_world.getActiveComponent<Health>(entity);

           if (shield == nullptr || health == nullptr)
               continue;
           // apply the rule here
       }
   }

   extern "C" {
       OrByte::DynamicLibrary::PluginType type()
       {
           return OrByte::DynamicLibrary::PluginType::SYSTEMS;
       }

       OrByte::ISystem *factory(World& world)
       {
           return new OrByte::ShieldSystem(world);
       }
   }
   ```

   `getExistingEntities()` returns a copy, so destroying entities inside the loop is safe. Do not keep component pointers after destroying an entity or removing that component.
3. Add `Shield` to `SYSTEM_PLUGINS` in `CMakeLists.txt`. The source must be named `<Name>System.cpp`.
4. In the compiled game, `server::GameInstance` (`src/GameEngine/server/game/GameInstance.cpp`) loads it automatically. Systems run in the order of `TICK_ORDER`, then the unlisted ones: add the name there when the order matters, for example before `Health`.

`ASystem` gives empty defaults for the optional parts of `ISystem`:

- `onEvent(const Event&)` receives the player input (keys, gamepad) before the tick. `ControlSystem` uses it.
- `getEntityPairs()` returns pairs of entity ids. `ColliderSystem` returns its collisions this way, and the game passes them to the scripts.

`factory(World&)` is the default signature. A system that needs another dependency gets a dedicated case in `GameInstance::createSystem`: `Display` and `Audio` receive the `IGraphic&`.

The multiplayer R-Type does not load systems by list: `src/server/game/GameInstance.cpp` and `src/client/scenes/AScene.cpp` load the ones they need by name.

### Existing system responsibilities

- `ControlSystem`: moves `Controllable` entities from keyboard and gamepad input.
- `ScriptSystem`: runs the Lua scripts of `Script` components (see the [scripting guide](scripting.md)).
- `PhysicSystem`: integrates `Velocity`, `Sinus`, `Orbit`, and `Spiral` into `Transform` positions.
- `ColliderSystem`: returns pairs of colliding ids; it does not decide what a collision means.
- `HealthSystem`: applies health-related components and destroys entities at zero health.
- `AudioSystem`: plays `Music` components through `IGraphic`.
- `DisplaySystem`: renders `Sprite`, `Animator` and `Controllable` components through `IGraphic`, relative to the `Camera`.

For a mechanic that is only needed by the R-Type server, add the rule to the server/game layer rather than writing a general engine system.

## Adding a compile feature

Features are optional parts of the compiled game, ticked in the Compile Options tab. `Chat` and `Lagometer` are the existing ones.

1. Add the name to `FEATURES` in `src/GameEngine/Console/CompileOptions.cpp`.
2. Guard the code with `#ifdef ORBYTE_FEATURE_<NAME>` (the name in upper case).

The define is only set on the game binary, so the guarded code must live in its sources: `src/GameEngine/client/` and `src/GameEngine/server/`. Plugins and the editor never see it.

## Adding a graphic library

A graphic library implements `IGraphic` (`include/plugins/IGraphic.hpp`): window, events, drawing, regions for the editor, cursor, and audio. Start from `AGraphic`, which keeps the window state.

1. Write the implementation in `src/plugins/`, exporting `type()` (returning `GRAPHICAL_LIB`) and `factory()` (returning a new `IGraphic*`).
2. Build it as a `MODULE` next to `rtype_sfml` in `CMakeLists.txt` and in `cmake/GameEngine/CMakeLists.txt`.
3. Select it in the editor through `GameEngine::GraphicalLibs` and `getGraphicalPlugin` (`src/GameEngine/GameEngine.cpp`). The compiled game loads `SFML` in `src/GameEngine/client/Client.cpp`.

Events reach the program as `OrByte::Event` (`include/GameEngine/Event.hpp`): convert the library's keys and buttons to it so systems never depend on the library.

## Extending the protocol safely

Read [protocol.md](protocol.md) before editing packet layouts. For a new message or field:

1. Add a stable enum value or payload field in `include/protocol/`.
2. Update the server-side encoder and parser/handler.
3. Update the matching client encoder and decoder.
4. Validate exact payload sizes before reading fields; keep little-endian encoding consistent.
5. Update the protocol reference and account for UDP packet loss. Do not rely on an event to keep the client world synchronized; snapshots carry authoritative entity state.

The current protocol has no negotiation for changing an existing packet layout, so server and client should be updated together.
