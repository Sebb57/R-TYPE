# Scripting guide

Entities are scripted in Lua 5.4 through the `Script` component and the `Script` system (`src/GameEngine/Systems/ScriptSystem.cpp`). `scripts/test.lua` is a short tour of the API; `scripts/starfall/` is a full game written with it.

## Attach a script

1. Write the script under `scripts/`, for example `scripts/mygame/enemy.lua`.
2. In the editor, add a `Script` component to the entity and set its path relative to `scripts/`: `mygame/enemy.lua`.
3. Compile the game with the `Script` system and component ticked.

Paths are resolved from the working directory, so run the game from the repository root. A script is reloaded when its file changes on disk: save it and the running game picks it up.

Each entity runs its own copy of the script. Top-level `local` variables are private to that entity; use `_G` for state shared between scripts.

## Callbacks

| Function | Called |
|---|---|
| `update(entity, dt)` | Every tick. Required: a script without it is rejected |
| `onCollisionEnter(entity, other)` | Once when `entity` starts touching `other`. Optional. Needs the `Collider` system |

`dt` is the time since the previous tick, in seconds.

When a callback raises an error, the game prints `[ScriptSystem] entity <id>: <error>` and stops running that script until the file is saved again.

```lua
function update(entity, dt)
    local transform = getComponent(entity, "Transform")
    if not transform then return end
    transform.position = Vector2.new(transform.position.x - 200 * dt, transform.position.y)
    if transform.position.x < -100 then destroyEntity(entity) end
end

function onCollisionEnter(entity, other)
    local name = getComponent(other, "Name")
    if name and name.name == "Player" then destroyEntity(entity) end
end
```

## API

| Function | Returns |
|---|---|
| `log(message)` | Prints `[lua] message` |
| `getEntities()` | Array of every entity id |
| `createEntity()` | New entity id (it already has a `Name`) |
| `duplicateEntity(id)` | Id of a copy with all its components, or `nil`. An entity holding a `Camera` cannot be copied |
| `destroyEntity(id)` | Nothing. The entity is removed once every callback of the tick returned |
| `getComponentsFrom(id)` | Array of the component names the entity has |
| `getComponent(id, name)` | The component, or `nil` when missing or disabled at compile time |
| `addComponent(id, name, fields)` | The component, or `nil`. `fields` is an optional table of initial values. An existing component is kept and updated, never replaced |
| `getComponentAttributes(name)` | Array of the field names of a component type |
| `setComponentAttribute(id, name, field, value)` | Sets one field. `...Int` and `...Str` variants force the type |
| `loadScene(path)` | `true`, or `false` and an error message. Replaces the world with an `.obe` file once the tick ends |

Components are returned by reference: assigning a field changes the entity directly.

```lua
local velocity = getComponent(entity, "Velocity")
if velocity then velocity.velocity = Vector2.new(0, -300) end
```

Do not keep a component across ticks: fetch it again in each callback, the entity may have been destroyed or the scene replaced.

`loadScene` takes a path from the working directory, for example `configs/stage_2.obe`. Use `_G` to carry data to the next scene.

## Types and fields

`Vector2.new(x, y)` builds a vector (`x`, `y`). `RGBA` has `r`, `g`, `b`, `a`. Enums: `ColliderShape.Box` / `ColliderShape.Circle`, `Team.Player` / `Team.Enemy`, `MusicState.Playing` / `MusicState.Paused` / `MusicState.Stopped`.

| Component | Fields |
|---|---|
| `Transform` | `position`, `scale`, `rotation` |
| `Velocity` | `velocity` |
| `Sprite` | `texturePath`, `size` |
| `Animator` | `currentAnimation`, `currentFrame`, `elapsed`, `playing` |
| `Collider` | `size`, `shape` |
| `Health` | `currentHealth`, `maxHealth` |
| `AddHealth` | `regenAmount` |
| `RemoveHealth` | `hitDamage` |
| `RegenHealth`, `Poison`, `Wither` | `regenAmount` or `damage`, `cooldown`, `duration` |
| `Orbit` | `center`, `targetEntity`, `radius`, `angle`, `angleSpeed` |
| `Sinus` | `center`, `targetEntity`, `distance`, `curr`, `speed` |
| `Spiral` | `center`, `targetEntity`, `radius`, `expandSpeed`, `angle`, `angleSpeed` |
| `Controllable` | `speed`, `size`, `color`, `image`, `boundX`, `boundY` |
| `Camera` | `boundX`, `boundY` |
| `Music` | `path`, `volume`, `pitch`, `loop`, `state` |
| `Owner` | `team` |
| `Name` | `name` |
| `Script` | `path` (read-only) |

Key bindings are not exposed: player movement goes through the `Controllable` component and the `Control` system.

## Patterns from Starfall

- **Templates.** Keep a hidden model entity in the scene (for example `Modele_Explosion_Joueur`), find it by `Name`, and `duplicateEntity` it instead of building entities field by field.
- **Short-lived entities.** Give the copy its own script that destroys it after a delay, like `starfall/explosion.lua` and `starfall/sound.lua`.
- **Game flow.** One controller entity runs the rules (`starfall/game.lua`) and switches stages with `loadScene`.
