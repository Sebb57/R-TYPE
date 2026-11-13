/**
 * @file Chat.hpp
 * @brief chat lines shared by the host, its server and the spectators
 *
 * @author Seb
 * @date 2026-11-13
**/

#ifndef GAMEENGINE_PROTOCOL_CHAT_HPP
#define GAMEENGINE_PROTOCOL_CHAT_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace server {

constexpr std::size_t MaxChatLength = 120;
constexpr std::size_t ChatHistory = 50;

struct ChatLine {
    std::string author;
    std::string text;
    bool host = false;
};

struct ChatLog {
    std::vector<ChatLine> lines;
    std::uint64_t total = 0;

    void add(ChatLine line)
    {
        if (this->lines.size() >= ChatHistory)
            this->lines.erase(this->lines.begin());
        this->lines.push_back(std::move(line));
        ++this->total;
    }
};

[[nodiscard]] inline bool isChatCharacter(unsigned character)
{
    return character >= ' ' && character <= '~';
}

[[nodiscard]] inline bool isChatText(std::string_view text)
{
    return !text.empty() && text.size() <= MaxChatLength && text.find_first_not_of(' ') != std::string_view::npos
        && std::ranges::all_of(text, [](char character) { return isChatCharacter(static_cast<unsigned char>(character)); });
}

} // namespace server

#endif // GAMEENGINE_PROTOCOL_CHAT_HPP
