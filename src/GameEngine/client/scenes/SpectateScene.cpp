/**
 * @file SpectateScene.cpp
 * @brief spectator scene implementation
 *
 * @author Seb
 * @date 2026-10-23
**/

#include "GameEngine/client/scenes/SpectateScene.hpp"
#include <algorithm>
#include <unordered_set>
#ifdef ORBYTE_FEATURE_CHAT
    #include <utility>
#endif
#ifdef ORBYTE_FEATURE_LAGOMETER
    #include <cstddef>
    #include <format>
#endif
#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/client/network/NetworkClient.hpp"
#include "GameEngine/client/scenes/RoomListScene.hpp"
#include "GameEngine/client/scenes/SceneManager.hpp"
#include "GameEngine/client/scenes/ServerScene.hpp"
#include "GameEngine/client/scenes/Theme.hpp"

namespace {

constexpr double StaleSnapshot = 0.25;

#ifdef ORBYTE_FEATURE_LAGOMETER
namespace lagometer {

constexpr float Width = 360.0F;
constexpr float Height = 160.0F;
constexpr float Padding = 12.0F;
constexpr float GraphTop = 64.0F;
constexpr float MinScale = 100.0F;
constexpr float GoodPing = 50.0F;
constexpr float SlowPing = 100.0F;

constexpr OrByte::RGBA Good(110, 200, 120);
constexpr OrByte::RGBA Slow(230, 190, 90);
constexpr OrByte::RGBA Bad(230, 110, 100);

[[nodiscard]] OrByte::RGBA colorOf(float rtt)
{
    if (rtt == client::NetworkMetrics::Lost || rtt >= SlowPing)
        return Bad;
    return rtt >= GoodPing ? Slow : Good;
}

} // namespace lagometer
#endif

} // namespace

client::SpectateScene::SpectateScene(SceneContext& context)
    : AScene(context)
#ifdef ORBYTE_FEATURE_CHAT
    , _chat(context, [&context](std::string text) { context.network.say(std::move(text)); })
#endif
{
    this->bindAction(Action::Back, [this](bool pressed) {
        if (pressed)
            this->_context.network.leave();
    });
    this->_view = this->_context.network.view();
}

#ifdef ORBYTE_FEATURE_CHAT
void client::SpectateScene::onEvent(const OrByte::Event& event)
{
    if (!this->_chat.onEvent(event))
        AScene::onEvent(event);
}
#endif

void client::SpectateScene::update(double deltaTime)
{
    AScene::update(deltaTime);
    this->_view = this->_context.network.view();
#ifdef ORBYTE_FEATURE_CHAT
    this->_chat.setAvailable(this->_view.chatAvailable);
    this->_chat.update(deltaTime, this->_view.chat);
#endif
    if (!this->_view.connected) {
        this->_context.scenes.switchTo(ServerScene::Name);
        return;
    }
    if (!this->_view.spectating) {
        this->_context.scenes.switchTo(RoomListScene::Name);
        return;
    }
    if (this->_view.tick != this->_tick) {
        this->_tick = this->_view.tick;
        this->_sinceSnapshot = 0.0;
        this->applySnapshot();
        return;
    }
    this->_sinceSnapshot += deltaTime;
    if (this->_sinceSnapshot >= StaleSnapshot)
        this->freeze();
}

void client::SpectateScene::render()
{
    AScene::render();
    this->drawText("SPECTATING  " + this->roomName(), Vector2(40.0F, 30.0F), 28, theme::Accent);
    if (this->_tick == 0)
        this->drawText(this->_view.status, Vector2(40.0F, 75.0F), 22, theme::Muted);
    this->drawText(this->_context.controls.keyName(Action::Back) + "  leave", Vector2(40.0F, 1020.0F), 22, theme::Muted);
#ifdef ORBYTE_FEATURE_LAGOMETER
    this->drawLagometer();
#endif
#ifdef ORBYTE_FEATURE_CHAT
    this->_chat.render(this->_view.chat);
#endif
}

void client::SpectateScene::applySnapshot()
{
    const auto rate = static_cast<float>(this->_view.snapshotRate);
    std::unordered_set<std::uint32_t> alive;
    for (const EntityView& remote : this->_view.entities) {
        alive.insert(remote.id);
        auto local = this->_entities.find(remote.id);
        if (local == this->_entities.end()) {
            const int entity = this->_world.createEntity();
            this->_world.addComponentToEntity<OrByte::Transform>(entity, remote.position, Vector2(1.0F, 1.0F), remote.rotation);
            this->_world.addComponentToEntity<OrByte::Velocity>(entity);
            this->_world.addComponentToEntity<OrByte::Controllable>(entity);
            local = this->_entities.emplace(remote.id, entity).first;
        }
        auto *transform = this->_world.getComponent<OrByte::Transform>(local->second);
        auto *velocity = this->_world.getComponent<OrByte::Velocity>(local->second);
        auto *controllable = this->_world.getComponent<OrByte::Controllable>(local->second);
        if (transform == nullptr || velocity == nullptr || controllable == nullptr)
            continue;
        transform->rotation = remote.rotation;
        velocity->velocity = Vector2((remote.position.x - transform->position.x) * rate, (remote.position.y - transform->position.y) * rate);
        controllable->size = remote.size;
        controllable->color = remote.color;
        controllable->image = remote.image;
    }
    for (auto entity = this->_entities.begin(); entity != this->_entities.end();) {
        if (alive.contains(entity->first)) {
            ++entity;
            continue;
        }
        this->_world.destroyEntity(entity->second);
        entity = this->_entities.erase(entity);
    }
}

void client::SpectateScene::freeze()
{
    for (const auto& [id, entity] : this->_entities)
        if (auto *velocity = this->_world.getComponent<OrByte::Velocity>(entity))
            velocity->velocity = Vector2(0.0F, 0.0F);
}

std::string client::SpectateScene::roomName() const
{
    const auto room = std::ranges::find(this->_view.rooms, this->_view.room, &RoomView::id);
    return room != this->_view.rooms.end() ? room->name : "#" + std::to_string(this->_view.room);
}

#ifdef ORBYTE_FEATURE_LAGOMETER
void client::SpectateScene::drawLagometer() const
{
    namespace lag = lagometer;
    const NetworkMetrics& metrics = this->_view.metrics;
    const Vector2 origin(theme::Width - lag::Width - 40.0F, 30.0F);
    const float graphHeight = lag::Height - lag::GraphTop - lag::Padding;
    const float graphWidth = lag::Width - (2.0F * lag::Padding);
    const float graphBottom = origin.y + lag::Height - lag::Padding;

    this->drawPanel(origin, Vector2(lag::Width, lag::Height), theme::Panel, 1, theme::PanelOutline);
    if (metrics.received == 0) {
        this->drawText(metrics.sent == 0 ? "PING  waiting..." : "PING  no answer", Vector2(origin.x + lag::Padding, origin.y + 8.0F),
            20, metrics.sent == 0 ? theme::Muted : lag::Bad);
    } else {
        this->drawText(std::format("PING  {:.0f} ms", metrics.smoothed), Vector2(origin.x + lag::Padding, origin.y + 8.0F), 20,
            lag::colorOf(metrics.smoothed));
        this->drawText(std::format("jitter {:.1f}", metrics.jitter), Vector2(origin.x + 200.0F, origin.y + 12.0F), 16, theme::Muted);
    }
    const float loss = metrics.sent == 0 ? 0.0F : 100.0F * static_cast<float>(metrics.lost) / static_cast<float>(metrics.sent);
    this->drawText(std::format("loss {}/{} ({:.1f}%)", metrics.lost, metrics.sent, loss),
        Vector2(origin.x + lag::Padding, origin.y + 36.0F), 16, metrics.lost == 0 ? theme::Muted : lag::Bad);

    float scale = lag::MinScale;
    for (const float sample : metrics.samples)
        scale = std::max(scale, sample);
    this->drawText(std::format("{:.0f}", scale), Vector2(origin.x + lag::Width - lag::Padding - 40.0F, origin.y + 36.0F), 14,
        theme::Disabled);

    const float barWidth = graphWidth / static_cast<float>(NetworkMetrics::History);
    float x = origin.x + lag::Padding + graphWidth - (barWidth * static_cast<float>(metrics.samples.size()));
    for (const float sample : metrics.samples) {
        const float height = sample == NetworkMetrics::Lost ? graphHeight : std::max(1.0F, graphHeight * sample / scale);
        this->drawPanel(Vector2(x, graphBottom - height), Vector2(std::max(1.0F, barWidth - 1.0F), height), lag::colorOf(sample));
        x += barWidth;
    }
}
#endif
