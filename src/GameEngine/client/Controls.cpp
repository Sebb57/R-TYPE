/**
 * @file Controls.cpp
 * @brief rebindable key to action mapping implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "GameEngine/client/Controls.hpp"

namespace {

const std::unordered_map<OrByte::Event::Key, std::string> KeyNames{
    {OrByte::Event::Key::Escape, "ESCAPE"},
    {OrByte::Event::Key::Space, "SPACE"},
    {OrByte::Event::Key::Backspace, "BACKSPACE"},
    {OrByte::Event::Key::Enter, "ENTER"},
    {OrByte::Event::Key::Left, "LEFT"},
    {OrByte::Event::Key::Right, "RIGHT"},
    {OrByte::Event::Key::Up, "UP"},
    {OrByte::Event::Key::Down, "DOWN"},
    {OrByte::Event::Key::Delete, "DELETE"},
    {OrByte::Event::Key::Insert, "INSERT"},
};

} // namespace

client::Controls::Controls()
    : _keys{
        {Action::Up, OrByte::Event::Key::Up},
        {Action::Down, OrByte::Event::Key::Down},
        {Action::Left, OrByte::Event::Key::Left},
        {Action::Right, OrByte::Event::Key::Right},
        {Action::Fire, OrByte::Event::Key::Space},
        {Action::Confirm, OrByte::Event::Key::Enter},
        {Action::Back, OrByte::Event::Key::Escape},
    }
{}

void client::Controls::rebind(Action action, OrByte::Event::Key key)
{
    const OrByte::Event::Key previous = this->_keys.at(action);
    for (auto& [other, bound] : this->_keys)
        if (bound == key)
            bound = previous;
    this->_keys[action] = key;
}

std::optional<client::Action> client::Controls::actionOf(OrByte::Event::Key key) const
{
    for (const auto& [action, bound] : this->_keys)
        if (bound == key)
            return action;
    return std::nullopt;
}

OrByte::Event::Key client::Controls::keyOf(Action action) const
{
    return this->_keys.at(action);
}

std::string client::Controls::keyName(Action action) const
{
    const OrByte::Event::Key key = this->keyOf(action);
    if (key >= OrByte::Event::Key::A && key <= OrByte::Event::Key::Z)
        return {1, static_cast<char>('A' + static_cast<int>(key) - static_cast<int>(OrByte::Event::Key::A))};
    const auto name = KeyNames.find(key);
    return name != KeyNames.end() ? name->second : "?";
}
