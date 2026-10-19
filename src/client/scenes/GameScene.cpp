/**
 * @file GameScene.cpp
 * @brief in game scene implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "client/scenes/GameScene.hpp"
#include <algorithm>
#include <filesystem>
#include <unordered_set>
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "Orbyte/Bonus.hpp"
#include "Orbyte/Ennemies.hpp"
#include "Orbyte/Playeur.hpp"
#include "Orbyte/Projectiles.hpp"
#include "client/network/NetworkClient.hpp"
#include "client/scenes/RoomScene.hpp"
#include "client/scenes/SceneManager.hpp"
#include "client/scenes/Theme.hpp"
#include "constants.hpp"

namespace {

constexpr std::size_t Shot1Sound = 0;
constexpr std::size_t Shot2Sound = 1;
constexpr std::size_t HitSound = 2;
constexpr std::size_t ExplosionSound = 3;

} // namespace

client::GameScene::GameScene(SceneContext& context)
    : AScene(context)
    , _inputs{
        {Action::Up, server::InputKey::Up},
        {Action::Down, server::InputKey::Down},
        {Action::Left, server::InputKey::Left},
        {Action::Right, server::InputKey::Right},
        {Action::Fire, server::InputKey::Fire},
    }
{
    constexpr const char *soundPaths[] = {
        "./assets/sounds/Assets_Sound_Fx_shot1.wav",
        "./assets/sounds/Assets_Sound_Fx_shot2.wav",
        "./assets/sounds/Assets_Sound_Fx_hit.wav",
        "./assets/sounds/Assets_Sound_Fx_explosion.wav",
    };
    for (std::size_t sound = 0; sound < this->_sounds.size(); ++sound) {
        if (this->_soundBuffers[sound].loadFromFile(soundPaths[sound])) {
            this->_sounds[sound].setBuffer(this->_soundBuffers[sound]);
            this->_sounds[sound].setVolume(75.0f);
        }
    }
    if (this->_music.openFromFile("./assets/sounds/backmusic.mp3")) {
        this->_music.setLoop(true);
        this->_music.setVolume(35.0f);
        this->_music.play();
    }
    constexpr float backgroundHeight = OrByte::constants::SERVER_WINDOW_Y;
    constexpr float backgroundWidth = backgroundHeight * (4224.0f / 240.0f);
    this->_backgroundEntity = this->_world.createEntity();
    this->_world.addComponentToEntity<OrByte::Transform>(this->_backgroundEntity,
        Vector2(backgroundWidth / 2.0f, backgroundHeight / 2.0f), Vector2(1.0f, 1.0f), 0.0f);
    this->_world.addComponentToEntity<OrByte::Velocity>(this->_backgroundEntity,
        Vector2(-backgroundWidth / 180.0f, 0.0f));
    this->_world.addComponentToEntity<OrByte::Sprite>(this->_backgroundEntity,
        "./assets/sprites/Background.png", Vector2(backgroundWidth, backgroundHeight));
    for (const auto& [action, input] : this->_inputs)
        this->bindInput(action, input);
    this->bindAction(Action::Back, [this](bool pressed) {
        if (pressed)
            this->_context.network.leaveRoom();
    });
}

client::GameScene::~GameScene()
{
    this->_context.network.setKeys(0);
    this->_music.stop();
    for (sf::Sound& sound : this->_sounds)
        sound.stop();
}

void client::GameScene::update(double deltaTime)
{
    AScene::update(deltaTime);
    this->_view = this->_context.network.view();
    this->updateCollisionEffects(deltaTime);
    for (auto explosion = this->_explosions.begin(); explosion != this->_explosions.end();) {
        const auto *animator = this->_world.getComponent<OrByte::Animator>(*explosion);
        if (animator != nullptr && animator->playing) {
            ++explosion;
            continue;
        }
        this->_world.destroyEntity(*explosion);
        explosion = this->_explosions.erase(explosion);
    }
    if (!this->_view.running) {
        this->_context.scenes.switchTo(RoomScene::Name);
        return;
    }
    if ((this->_keys & static_cast<std::uint8_t>(server::InputKey::Fire)) != 0) {
        this->_shotSoundTimer -= deltaTime;
        if (this->_shotSoundTimer <= 0.0) {
            this->playSound(this->_nextShotSound == 0 ? Shot1Sound : Shot2Sound);
            this->_nextShotSound = 1 - this->_nextShotSound;
            this->_shotSoundTimer = 1.0 / OrByte::constants::FIRE_RATE;
        }
    } else {
        this->_shotSoundTimer = 0.0;
    }
    for (const CollisionEffectView& effect : this->_view.collisionEffects) {
        if (effect.eventId <= this->_lastCollisionEvent)
            continue;
        if (effect.type == static_cast<std::uint8_t>(server::CollisionEffect::EnemyDestroyed)
            || effect.type == static_cast<std::uint8_t>(server::CollisionEffect::PlayerDestroyed)) {
            if (effect.type == static_cast<std::uint8_t>(server::CollisionEffect::EnemyDestroyed))
                this->playSound(ExplosionSound);
            this->spawnExplosion(Vector2(static_cast<float>(effect.x), static_cast<float>(effect.y)));
        } else {
            if (effect.type == static_cast<std::uint8_t>(server::CollisionEffect::RedEnemy)
                || effect.type == static_cast<std::uint8_t>(server::CollisionEffect::RedPlayer))
                this->playSound(HitSound);
            this->addCollisionEffect(effect);
        }
        this->_lastCollisionEvent = effect.eventId;
    }
    if (this->_view.tick == this->_tick)
        return;
    this->_tick = this->_view.tick;
    this->applySnapshot(this->_view.entities);
    if (this->_view.gameOver) {
        if (this->_music.getStatus() == sf::SoundSource::Playing)
            this->_music.stop();
        if (auto *velocity = this->_world.getComponent<OrByte::Velocity>(this->_backgroundEntity))
            velocity->velocity = Vector2(0.0f, 0.0f);
    }
}

void client::GameScene::render()
{
    AScene::render();
    const unsigned minutes = this->_view.remainingSeconds / 60;
    const unsigned seconds = this->_view.remainingSeconds % 60;
    const std::string clock = "Time " + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
    this->drawText(clock, Vector2(40.0f, 30.0f), 28, theme::Text);

    const bool playerAlive = this->_view.slot < 4 && (this->_view.aliveMask & (1U << this->_view.slot)) != 0;
    const bool showScoreboard = this->_view.gameOver || (this->_view.hasGameState && !playerAlive);
    if (!showScoreboard) {
        if (this->_view.slot < 4)
            this->drawText("Score " + std::to_string(this->_view.scores[this->_view.slot]), Vector2(40.0f, 68.0f), 22, theme::Muted);
        return;
    }

    this->drawPanel(Vector2(0.0f, 0.0f), Vector2(theme::Width, theme::Height), {0, 0, 0, 190});
    this->drawPanel(Vector2(430.0f, 250.0f), Vector2(1060.0f, 580.0f), theme::Panel, 2, theme::PanelOutline);
    this->drawText(this->_view.gameOver ? "GAME OVER" : "SHIP DESTROYED", Vector2(805.0f, 315.0f), 48, theme::Accent);
    this->drawText("SCOREBOARD", Vector2(850.0f, 390.0f), 28, theme::Text);
    float rowY = 465.0f;
    for (std::uint8_t slot = 0; slot < 4; ++slot) {
        const auto player = std::find_if(this->_view.players.begin(), this->_view.players.end(),
            [slot](const PlayerView& entry) { return entry.slot == slot; });
        if (player == this->_view.players.end() && this->_view.scores[slot] == 0)
            continue;
        const std::string name = player == this->_view.players.end()
            ? "Player " + std::to_string(slot + 1) : player->name;
        const bool alive = (this->_view.aliveMask & (1U << slot)) != 0;
        this->drawText("P" + std::to_string(slot + 1) + "  " + name + (alive ? "" : "  (dead)"),
            Vector2(650.0f, rowY), 28, theme::Slots[slot]);
        this->drawText(std::to_string(this->_view.scores[slot]), Vector2(1300.0f, rowY), 28, theme::Text);
        rowY += 58.0f;
    }
    this->drawText(this->_view.gameOver ? "Press Escape to quit" : "Press Escape to leave, or wait for the game to end",
        Vector2(this->_view.gameOver ? 815.0f : 620.0f, 760.0f), 23, theme::Muted);
}

void client::GameScene::bindInput(Action action, server::InputKey input)
{
    this->bindAction(action, [this, input](bool pressed) {
        const auto bit = static_cast<std::uint8_t>(input);
        if (pressed)
            this->_keys |= bit;
        else
            this->_keys &= static_cast<std::uint8_t>(~bit);
        this->_context.network.setKeys(this->_keys);
    });
}

void client::GameScene::applySnapshot(const std::vector<EntityView>& entities)
{
    const auto rate = static_cast<float>(this->_view.snapshotRate);
    std::unordered_set<std::uint32_t> alive;
    for (const EntityView& remote : entities) {
        alive.insert(remote.id);
        const Vector2 position(static_cast<float>(remote.x), static_cast<float>(remote.y));
        const auto local = this->_entities.find(remote.id);
        if (local == this->_entities.end()) {
            this->_entities.emplace(remote.id, this->spawn(static_cast<server::EntityType>(remote.type), position, remote.flags));
            this->_previousPositions.insert_or_assign(remote.id, position);
            continue;
        }
        const auto previous = this->_previousPositions.find(remote.id);
        if (remote.type == static_cast<std::uint8_t>(server::EntityType::Player) && previous != this->_previousPositions.end()) {
            const float deltaY = position.y - previous->second.y;
            this->setPlayerAnimation(local->second, deltaY < -1.0f ? "moveup" : deltaY > 1.0f ? "movedown" : "moveforward");
        }
        const auto *transform = this->_world.getComponent<OrByte::Transform>(local->second);
        auto *velocity = this->_world.getComponent<OrByte::Velocity>(local->second);
        if (transform != nullptr && velocity != nullptr)
            velocity->velocity = Vector2((position.x - transform->position.x) * rate, (position.y - transform->position.y) * rate);
        this->_previousPositions.insert_or_assign(remote.id, position);
    }
    for (auto entity = this->_entities.begin(); entity != this->_entities.end();) {
        if (alive.contains(entity->first)) {
            ++entity;
            continue;
        }
        const auto flash = std::find_if(this->_collisionFlashes.begin(), this->_collisionFlashes.end(),
            [&entity](const CollisionFlash& entry) { return entry.entity == entity->second; });
        if (flash != this->_collisionFlashes.end()) {
            if (auto *velocity = this->_world.getComponent<OrByte::Velocity>(entity->second))
                velocity->velocity = Vector2(0.0f, 0.0f);
            ++entity;
            continue;
        }
        this->_world.destroyEntity(entity->second);
        this->_playersWhoPlayedIdle.erase(entity->second);
        this->_previousPositions.erase(entity->first);
        entity = this->_entities.erase(entity);
    }
}

void client::GameScene::setPlayerAnimation(int entity, const std::string& animation)
{
    auto *animator = this->_world.getComponent<OrByte::Animator>(entity);
    if (animator == nullptr)
        return;
    if (animation == "movedown" && !this->_playersWhoPlayedIdle.contains(entity)) {
        if (animator->currentAnimation != "moveforward") {
            animator->play("moveforward");
            return;
        }
        if (animator->playing)
            return;
        this->_playersWhoPlayedIdle.insert(entity);
    }
    if (animator->currentAnimation != animation)
        animator->play(animation);
}

int client::GameScene::spawn(server::EntityType type, Vector2 position, std::uint8_t flags)
{
    int entity = -1;
    if (type == server::EntityType::Player) {
        RType::Playeur player(&this->_world);
        player.CreateClientComponent(&this->_world, static_cast<std::uint8_t>(flags & server::EntitySlotMask));
        entity = player.GetID();
    } else if (type == server::EntityType::Enemy) {
        RType::Ennemies enemy(&this->_world);
        enemy.CreateClientComponent(&this->_world);
        entity = enemy.GetID();
    } else if (type == server::EntityType::Projectile) {
        RType::Projectiles projectile(&this->_world, -1, Team::Player);
        projectile.CreateClientComponent(&this->_world);
        entity = projectile.GetID();
    } else if (type == server::EntityType::Bonus) {
        RType::Bonus bonus(&this->_world);
        bonus.CreateClientBonus(&this->_world, (flags & server::BonusExtraShotFlag) != 0);
        entity = bonus.GetID();
    } else {
        entity = this->_world.createEntity();
    }
    this->_world.addComponentToEntity<OrByte::Transform>(entity, position, Vector2(1.0f, 1.0f), 0.0f);
    this->_world.addComponentToEntity<OrByte::Velocity>(entity);
    return entity;
}

void client::GameScene::spawnExplosion(Vector2 position)
{
    const int entity = this->_world.createEntity();
    this->_world.addComponentToEntity<OrByte::Transform>(entity, position, Vector2(1.0f, 1.0f), 0.0f);
    OrByte::Animation animation;
    animation.spriteSheetPath = "./assets/sprites/explosion.png";
    animation.isSpriteSheet = true;
    animation.frameCount = 8;
    animation.frameSize = Vector2(271.5f, 724.0f);
    animation.displaySize = Vector2(110.0f, 294.0f);
    animation.columns = 8;
    animation.frameDuration = 0.07f;
    animation.loop = false;
    OrByte::Animator animator;
    animator.addAnimation("explosion", animation);
    animator.play("explosion");
    this->_world.addComponentToEntity<OrByte::Animator>(entity, animator);
    this->_explosions.push_back(entity);
}

void client::GameScene::updateCollisionEffects(double deltaTime)
{
    for (auto flash = this->_collisionFlashes.begin(); flash != this->_collisionFlashes.end();) {
        flash->elapsed += deltaTime;
        if (auto *animator = this->_world.getComponent<OrByte::Animator>(flash->entity)) {
            const bool tinted = static_cast<unsigned>(flash->elapsed / 0.08) % 2U == 0;
            for (auto& [name, animation] : animator->animations) {
                (void)name;
                animation.spriteSheetPath = tinted ? flash->flashAsset : flash->originalAsset;
            }
        }
        if (flash->elapsed >= 0.48) {
            if (auto *animator = this->_world.getComponent<OrByte::Animator>(flash->entity))
                for (auto& [name, animation] : animator->animations) {
                    (void)name;
                    animation.spriteSheetPath = flash->originalAsset;
                }
            flash = this->_collisionFlashes.erase(flash);
        } else {
            ++flash;
        }
    }
}

void client::GameScene::addCollisionEffect(const CollisionEffectView& effect)
{
    const auto entity = this->_entities.find(effect.entityId);
    if (entity == this->_entities.end())
        return;
    auto *animator = this->_world.getComponent<OrByte::Animator>(entity->second);
    const OrByte::Animation *animation = animator ? animator->getCurrentAnimation() : nullptr;
    if (animation == nullptr)
        return;
    auto flash = std::find_if(this->_collisionFlashes.begin(), this->_collisionFlashes.end(),
        [&entity](const CollisionFlash& entry) { return entry.entity == entity->second; });
    const std::string originalAsset = flash != this->_collisionFlashes.end()
        ? flash->originalAsset : animation->spriteSheetPath;
    const bool blue = effect.type == static_cast<std::uint8_t>(server::CollisionEffect::BluePlayer);
    std::filesystem::path tintedPath(originalAsset);
    tintedPath.replace_filename(tintedPath.stem().string() + (blue ? "_blue" : "_red") + tintedPath.extension().string());
    if (flash == this->_collisionFlashes.end()) {
        this->_collisionFlashes.push_back({entity->second, 0.0, originalAsset, tintedPath.string()});
    } else {
        flash->elapsed = 0.0;
        flash->flashAsset = tintedPath.string();
    }
}

void client::GameScene::playSound(std::size_t sound)
{
    if (sound >= this->_sounds.size() || this->_soundBuffers[sound].getSampleCount() == 0)
        return;
    this->_sounds[sound].stop();
    this->_sounds[sound].play();
}
