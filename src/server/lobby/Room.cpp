/**
 * @file Room.cpp
 * @brief room class implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include "server/lobby/Room.hpp"
#include <algorithm>
#include <utility>

server::Room::Room(RoomId id, std::string name, std::uint8_t maxPlayers, SessionToken host, OutgoingSink sink, const ServerConfig& conf)
    : _id(id)
    , _name(std::move(name))
    , _host(host)
    , _maxPlayers(maxPlayers)
    , _conf(conf)
    , _sink(std::move(sink))
{
    if (this->_maxPlayers > 0 && this->_maxPlayers <= this->_slots.size())
        this->_slots[0] = host;
}

std::optional<server::PlayerSlot> server::Room::addPlayer(SessionToken token)
{
    if (token == 0 || this->_state != RoomState::Lobby || this->isFull() || this->contains(token))
        return std::nullopt;
    for (PlayerSlot slot = 0; slot < this->_maxPlayers; ++slot) {
        if (!this->_slots[slot]) {
            this->_slots[slot] = token;
            this->_ready[slot] = false;
            return slot;
        }
    }
    return std::nullopt;
}

void server::Room::setReady(PlayerSlot slot, bool ready)
{
    if (slot < this->_maxPlayers && this->_slots[slot] && this->_state == RoomState::Lobby)
        this->_ready[slot] = ready;
}

bool server::Room::isReady(PlayerSlot slot) const
{
    return slot < this->_maxPlayers && this->_slots[slot].has_value() && this->_ready[slot];
}

bool server::Room::allReady() const
{
    bool found = false;
    for (PlayerSlot slot = 0; slot < this->_maxPlayers; ++slot) {
        if (!this->_slots[slot])
            continue;
        found = true;
        if (!this->_ready[slot])
            return false;
    }
    return found;
}

void server::Room::setPlayerName(PlayerSlot slot, std::string name)
{
    if (slot < this->_maxPlayers && this->_slots[slot])
        this->_playerNames[slot] = std::move(name);
}

void server::Room::removePlayer(PlayerSlot slot)
{
    if (slot >= this->_maxPlayers || !this->_slots[slot])
        return;
    SessionToken removed = *this->_slots[slot];
    this->_slots[slot].reset();
    this->_ready[slot] = false;
    if (removed == this->_host)
        this->promoteNextHost();
}

bool server::Room::isFull() const
{
    return this->getInfo().players >= this->_maxPlayers;
}

bool server::Room::isEmpty() const
{
    return this->getInfo().players == 0;
}

bool server::Room::contains(SessionToken token) const
{
    return this->slotOf(token).has_value();
}

std::optional<server::PlayerSlot> server::Room::slotOf(SessionToken token) const
{
    for (PlayerSlot slot = 0; slot < this->_maxPlayers; ++slot)
        if (this->_slots[slot] == token)
            return slot;
    return std::nullopt;
}

void server::Room::promoteNextHost()
{
    this->_host = 0;
    for (const auto& player : this->_slots) {
        if (player) {
            this->_host = *player;
            break;
        }
    }
}

void server::Room::start()
{
    if (this->_state != RoomState::Lobby || this->isEmpty() || !this->allReady())
        return;
    this->_game = std::make_unique<GameInstance>(this->_id, this->_conf.tickRate, this->_conf.snapshotRate, this->_sink);
    this->_state = RoomState::Running;
    for (PlayerSlot slot = 0; slot < this->_maxPlayers; ++slot) {
        if (this->_slots[slot])
            this->_game->push(PlayerJoined(slot, this->_playerNames[slot]));
    }
    this->_game->start();
}

void server::Room::stop()
{
    if (this->_game) {
        this->_game->stop();
        this->_game.reset();
    }
    if (this->_state == RoomState::Running)
        this->_state = RoomState::Finished;
}

void server::Room::push(InboundEvent event)
{
    if (this->_state == RoomState::Running && this->_game)
        this->_game->push(std::move(event));
}

server::RoomInfo server::Room::getInfo() const
{
    const auto players = static_cast<std::uint8_t>(std::count_if(this->_slots.begin(), this->_slots.begin() + this->_maxPlayers,
        [](const auto& slot) { return slot.has_value(); }));
    return RoomInfo(this->_id, this->_name, players, this->_maxPlayers, this->_state);
}

void server::Room::refresh()
{
    if (this->_state == RoomState::Running && this->_game && this->_game->finished()) {
        this->_game->stop();
        this->_game.reset();
        this->_state = RoomState::Finished;
    }
}
