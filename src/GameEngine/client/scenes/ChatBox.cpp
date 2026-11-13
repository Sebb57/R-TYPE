/**
 * @file ChatBox.cpp
 * @brief chat overlay implementation
 *
 * @author Seb
 * @date 2026-11-13
**/

#ifdef ORBYTE_FEATURE_CHAT

    #include "GameEngine/client/scenes/ChatBox.hpp"
    #include <algorithm>
    #include <optional>
    #include <ranges>
    #include <utility>
    #include "GameEngine/ECS/Components/Transform.hpp"
    #include "GameEngine/client/scenes/Theme.hpp"

namespace {

constexpr float Left = 40.0F;
constexpr float Width = 680.0F;
constexpr float Bottom = 1005.0F;
constexpr float Padding = 10.0F;
constexpr float RowHeight = 26.0F;
constexpr float HintX = 300.0F;
constexpr float HintY = 1020.0F;
constexpr unsigned TextSize = 20;
constexpr unsigned HintSize = 22;
constexpr std::size_t WrapColumns = 60;
constexpr std::size_t TypingRows = 10;
constexpr std::size_t IdleRows = 5;
constexpr double FadeAfter = 10.0;

constexpr OrByte::RGBA IdleFill(13, 18, 29, 140);
constexpr OrByte::RGBA TypingFill(13, 18, 29, 220);

struct Row {
    std::string text;
    OrByte::RGBA color;
};

} // namespace

std::vector<std::string> client::wrapChat(const std::string& text, std::size_t columns)
{
    std::vector<std::string> rows;
    std::string row;

    for (const auto part : std::views::split(text, ' ')) {
        std::string word(part.begin(), part.end());

        if (word.empty())
            continue;
        while (word.size() > columns) {
            const std::size_t room = row.empty() ? columns : columns - std::min(columns, row.size() + 1);

            if (room == 0) {
                rows.push_back(std::exchange(row, {}));
                continue;
            }
            if (!row.empty())
                row.push_back(' ');
            row += word.substr(0, room);
            word.erase(0, room);
            rows.push_back(std::exchange(row, {}));
        }
        if (word.empty())
            continue;
        if (!row.empty() && row.size() + 1 + word.size() > columns)
            rows.push_back(std::exchange(row, {}));
        if (!row.empty())
            row.push_back(' ');
        row += word;
    }
    if (!row.empty())
        rows.push_back(std::move(row));
    return rows;
}

client::ChatBox::ChatBox(SceneContext& context, Send send) : _context(context), _send(std::move(send)) {}

bool client::ChatBox::onEvent(const OrByte::Event& event)
{
    if (!this->_available)
        return false;
    const std::optional<Action> action = event.type == OrByte::Event::Type::KeyPressed
        ? this->_context.controls.actionOf(event.key) : std::nullopt;

    if (!this->_typing) {
        if (action != Action::Confirm)
            return false;
        this->_typing = true;
        return true;
    }
    if (event.type == OrByte::Event::Type::TextEntered) {
        if (event.keyChar == '\b' && !this->_input.empty())
            this->_input.pop_back();
        else if (server::isChatCharacter(event.keyChar) && this->_input.size() < server::MaxChatLength)
            this->_input.push_back(static_cast<char>(event.keyChar));
        return true;
    }
    if (event.type != OrByte::Event::Type::KeyPressed)
        return false;
    if (action == Action::Confirm && server::isChatText(this->_input))
        this->_send(this->_input);
    if (action == Action::Confirm || action == Action::Back)
        this->close();
    return true;
}

void client::ChatBox::update(double deltaTime, const server::ChatLog& log)
{
    if (log.total != this->_seen) {
        this->_seen = log.total;
        this->_idle = 0.0;
        return;
    }
    this->_idle += deltaTime;
}

void client::ChatBox::render(const server::ChatLog& log) const
{
    if (!this->_available)
        return;
    const Controls& controls = this->_context.controls;
    if (this->_typing)
        this->drawText(controls.keyName(Action::Confirm) + "  send        " + controls.keyName(Action::Back) + "  cancel",
            Vector2(HintX, HintY), HintSize, theme::Muted);
    else
        this->drawText(controls.keyName(Action::Confirm) + "  chat", Vector2(HintX, HintY), HintSize, theme::Muted);

    std::vector<Row> rows;
    if (this->_typing || this->_idle < FadeAfter) {
        const std::size_t maxRows = this->_typing ? TypingRows : IdleRows;
        for (const server::ChatLine& line : log.lines | std::views::reverse) {
            const std::vector<std::string> wrapped = wrapChat(line.author + ": " + line.text, WrapColumns);
            for (const std::string& text : wrapped | std::views::reverse) {
                if (rows.size() >= maxRows)
                    break;
                rows.push_back(Row{text, line.host ? theme::Accent : theme::Text});
            }
            if (rows.size() >= maxRows)
                break;
        }
        std::ranges::reverse(rows);
    }
    if (rows.empty() && !this->_typing)
        return;

    const float height = (static_cast<float>(rows.size()) + (this->_typing ? 1.0F : 0.0F)) * RowHeight + 2.0F * Padding;
    float y = Bottom - height;
    this->drawPanel(Vector2(Left, y), Vector2(Width, height), this->_typing ? TypingFill : IdleFill,
        this->_typing ? 1 : 0, theme::PanelOutline);
    y += Padding;
    for (const Row& row : rows) {
        this->drawText(row.text, Vector2(Left + Padding, y), TextSize, row.color, Width - 2.0F * Padding);
        y += RowHeight;
    }
    if (!this->_typing)
        return;
    const std::size_t visible = WrapColumns - 3;
    const std::string input = this->_input.size() > visible ? "..." + this->_input.substr(this->_input.size() - visible) : this->_input;
    this->drawText("> " + input + "_", Vector2(Left + Padding, y), TextSize, theme::Accent);
}

void client::ChatBox::setAvailable(bool available)
{
    this->_available = available;
    if (!available)
        this->close();
}

void client::ChatBox::close()
{
    this->_typing = false;
    this->_input.clear();
}

void client::ChatBox::drawText(const std::string& text, Vector2 position, unsigned size, OrByte::RGBA color, float maxWidth) const
{
    if (this->_context.font.empty())
        return;
    this->_context.graphic.drawText(OrByte::Transform(position, Vector2(1.0F, 1.0F), 0.0F), text, size, maxWidth, color, 0, {0, 0, 0},
        this->_context.font);
}

void client::ChatBox::drawPanel(Vector2 position, Vector2 size, OrByte::RGBA fill, unsigned outlineThickness, OrByte::RGBA outline) const
{
    const Vector2 center(position.x + size.x / 2.0F, position.y + size.y / 2.0F);
    this->_context.graphic.drawRectangle(OrByte::Transform(center, Vector2(1.0F, 1.0F), 0.0F), size, fill, outlineThickness, outline);
}

#endif
