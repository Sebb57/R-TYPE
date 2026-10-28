# Developer guide

## Build and run

Use the commands in the [README](../README.md). Run the client from the repository root: assets are loaded from relative `./assets/...` paths. The server listens on TCP `6969` and UDP `6767` by default; those values are in `server::ServerConfig`.

## Where to make a change

| Goal | Main location |
|---|---|
| Add or adjust server-authoritative gameplay | `src/server/game/GameInstance.cpp`, `include/server/game/GameInstance.hpp` |
| Define game entity components and actions | `src/Orbyte/` and `include/Orbyte/` |
| Add a client visual or animation | `src/client/scenes/GameScene.cpp` or the entity's `CreateClient...` method |
| Change the room/lobby lifecycle | `src/server/lobby/`, then the matching client scene under `src/client/scenes/` |
| Change network messages | `include/protocol/`, server serializers/handlers, and `src/client/network/NetworkClient.cpp` together |
| Add a reusable ECS component or system | `include/GameEngine/ECS/Components/` or `include/GameEngine/Systems/`, plus `src/GameEngine/` |

The server owns health, collisions, scoring, spawn timing, player input, and entity lifetime. Clients send held input and mirror server snapshots. Keep gameplay decisions on the server; use events for short-lived visual/audio feedback. For example, explosions are local client entities created from death events and have no collider.

## Adding an entity

1. Add the entity interface under `include/Orbyte/` and its implementation under `src/Orbyte/`.
2. Put server setup in its `CreateServeur...` method: create the collider, transform, velocity, health, and ownership components needed by the simulation.
3. Put client presentation in its `CreateClient...` method: use a Sprite or Animator and the corresponding asset. Do not add rendering-only colliders.
4. Register the type in `server::EntityType` and update `GameInstance::spawn`, snapshots, and the client `GameScene::spawn` when the network must represent it.
5. Add collision, cleanup, and out-of-bounds behavior to the server simulation where applicable.

Follow the existing entity pattern: entity methods own their component setup; the game loop decides when to construct, update, or destroy them. Keep the ECS core reusable and put game-specific policy in the game/server layer.

## Adding a gameplay mechanic

For a server-only rule, implement it in `GameInstance` or the relevant entity method. Entity behavior such as steering, firing, health changes, and bonus application already has entity methods; reuse those rather than assembling components in the game loop. If the client needs a visible effect, add an event type and send it through the existing event DTO. Decode it in `NetworkClient`, then handle presentation in `GameScene`.

## Adding a component to the Game Engine

A component is data attached to an entity id. Components inherit `IComponent`, live under `include/GameEngine/ECS/Components/`, and declare a unique `ComponentType::Type`. `ComponentManager` uses this value as the component storage key, so adding a component requires updating both the enum and the component class.

1. Add a unique enumerator to `ComponentType` in `include/GameEngine/ECS/Components/IComponent.hpp`.
2. Create a header, for example `include/GameEngine/ECS/Components/Shield.hpp`:

   ```cpp
   #pragma once
   #include "IComponent.hpp"

   namespace OrByte {
   struct Shield : IComponent {
       inline static constexpr ComponentType Type = ComponentType::Shield;
       float points = 0.0f;
   };
   }
   ```

3. Attach it where the entity is constructed, passing constructor arguments through `World`:

   ```cpp
   world->addComponentToEntity<OrByte::Shield>(entityId);
   ```

   Give the component a constructor if it needs initial values, then pass those constructor arguments to `addComponentToEntity`. Retrieve it with `world.getComponent<OrByte::Shield>(entityId)`; this returns `nullptr` when the entity does not have that component. Remove it with `world.removeComponentFromEntity<OrByte::Shield>(entityId)`. Destroying the entity through `World::destroyEntity` removes all its components automatically.

4. If the component is game-specific, define it outside `GameEngine/` instead; keep reusable engine data free of OrByte gameplay rules.

Use value-like data and avoid putting update loops or dependencies on client/server code into a component. `ComponentType` values must be unique. The engine currently stores components by this enum and entity id through `ComponentManager`.

## Adding a system to the Game Engine

A system contains behavior that operates on entities with certain components. There is no global system registry: the scene or game object owns its systems and explicitly calls their update methods. Existing examples are `PhysicSystem`, `ColliderSystem`, `HealthSystem`, and `DisplaySystem`.

1. Declare the system under `include/GameEngine/Systems/`. Keep a `World&` so it can query components. For example:

   ```cpp
   #pragma once
   #include "GameEngine/ECS/World.hpp"

   namespace OrByte {
   class ShieldSystem {
       World& world;
   public:
       explicit ShieldSystem(World& world) : world(world) {}
       void update();
   };
   }
   ```

2. Implement `update()` in `src/GameEngine/Systems/ShieldSystem.cpp`. Iterate a copy from `world.getExistingEntities()`, query the required components, and skip entities missing any required component:

   ```cpp
   for (int entity : world.getExistingEntities()) {
       auto* shield = world.getComponent<OrByte::Shield>(entity);
       auto* health = world.getComponent<OrByte::Health>(entity);
       if (!shield || !health)
           continue;
       // Apply the reusable rule here.
   }
   ```

   Iterating a returned entity list is safer if the system may destroy entities. Do not retain component pointers across entity destruction or changes to the corresponding component.

3. Add the system as a member next to the `World` it uses. Initialize it with that same world, then call `update()` at the right point in the frame/tick. The client scene base owns physics and display systems; the server `GameInstance` owns physics, collision, and health systems. Ordering matters: movement usually precedes collision checks; health/death handling follows collision damage; display runs during rendering.
4. The current CMake setup discovers `src/GameEngine/*.cpp` recursively, so a new system source is included automatically. Add any nonstandard target dependencies in `CMakeLists.txt` if the system needs them.

Keep engine systems reusable: depend on components and `World`, not on `GameScene`, `GameInstance`, assets, network DTOs, or game-specific entity classes. Put R-Type policy in the game layer and call into it from the game loop when needed.

### Existing system responsibilities

- `PhysicSystem`: integrates `Velocity`, `Sinus`, `Orbit`, and `Spiral` into `Transform` positions.
- `ColliderSystem`: returns pairs of colliding ids; it does not decide what a collision means.
- `HealthSystem`: applies health-related components and destroys entities at zero health.
- `DisplaySystem`: renders `Sprite` and `Animator` components through `IGraphic`.

For a mechanic that is only needed by the R-Type server, add the rule to the server/game layer rather than registering a general engine system.

## Extending the protocol safely

Read [protocol.md](protocol.md) before editing packet layouts. For a new message or field:

1. Add a stable enum value or payload field in `include/protocol/`.
2. Update the server-side encoder and parser/handler.
3. Update the matching client encoder and decoder.
4. Validate exact payload sizes before reading fields; keep little-endian encoding consistent.
5. Update the protocol reference and account for UDP packet loss. Do not rely on an event to keep the client world synchronized; snapshots carry authoritative entity state.

The current protocol has no negotiation for changing an existing packet layout, so server and client should be updated together.
