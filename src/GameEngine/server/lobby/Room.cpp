/**
 * @file Room.cpp
 * @brief room class implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include "GameEngine/server/lobby/Room.hpp"
#include <algorithm>
#include <utility>

server::Room::Room(RoomId id, std::string name, std::shared_ptr<GameInstance> game, std::uint8_t maxSpectators, OutgoingSink sink)
    : _id(id)
    , _name(std::move(name))
    , _game(std::move(game))
    , _maxSpectators(maxSpectators)
    , _sink(std::move(sink))
{}

server::Room::~Room()
{
    this->stream(false);
}

bool server::Room::addSpectator(SessionToken token)
{
    if (token == 0 || this->isFull() || std::ranges::find(this->_spectators, token) != this->_spectators.end())
        return false;
    this->_spectators.push_back(token);
    if (this->_spectators.size() == 1)
        this->stream(true);
    return true;
}

void server::Room::removeSpectator(SessionToken token)
{
    std::erase(this->_spectators, token);
    if (this->_spectators.empty())
        this->stream(false);
}

bool server::Room::isFull() const
{
    return this->_spectators.size() >= this->_maxSpectators;
}

bool server::Room::isFinished() const
{
    return this->_game->finished();
}

server::RoomInfo server::Room::getInfo() const
{
    return RoomInfo(this->_id, this->_name, static_cast<std::uint8_t>(this->_spectators.size()), this->_maxSpectators);
}

void server::Room::stream(bool enabled)
{
    if (!enabled) {
        this->_game->setSink(nullptr);
        return;
    }
    this->_game->setSink([sink = this->_sink, id = this->_id](SnapshotDto&& snapshot) {
        sink(OutboundMessage(id, std::move(snapshot)));
    });
}
