/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ConsoleWidget.hpp
*/

#pragma once
#ifndef CONSOLEWIDGET_HPP_
    #define CONSOLEWIDGET_HPP_

    #include "GameEngine/Console/Console.hpp"
    #include "Widget.hpp"
    #include <cstddef>
    #include <string>

namespace OrByte {

class ConsoleWidget final : public Widget {
    public:
        explicit ConsoleWidget(Console& console, int size = 0);
        ~ConsoleWidget() override = default;

        ConsoleWidget(const ConsoleWidget&) = delete;
        ConsoleWidget& operator=(const ConsoleWidget&) = delete;
        ConsoleWidget(ConsoleWidget&&) = delete;
        ConsoleWidget& operator=(ConsoleWidget&&) = delete;

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;

    private:
        Console& _console;
        std::string _input;
        std::size_t _scroll = 0;
        std::size_t _historyIndex = 0;

        bool _onKey(const Event& event);
        void _browseHistory(int direction);
};

} // namespace OrByte

#endif /* CONSOLEWIDGET_HPP_ */
