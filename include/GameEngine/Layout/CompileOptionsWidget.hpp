/**
 * @file CompileOptionsWidget.hpp
 * @brief picks the game binary name and the systems / components / features it is compiled with
 *
 * @author Seb
 * @date 2026-11-11
**/

#pragma once
#ifndef COMPILEOPTIONSWIDGET_HPP_
    #define COMPILEOPTIONSWIDGET_HPP_

    #include "GameEngine/Console/CompileOptions.hpp"
    #include "Widget.hpp"
    #include <cstdint>
    #include <string>
    #include <utility>
    #include <vector>

namespace OrByte {

class CompileOptionsWidget final : public Widget {
    public:
        explicit CompileOptionsWidget(int size = 0);
        ~CompileOptionsWidget() override = default;

        CompileOptionsWidget(const CompileOptionsWidget&) = delete;
        CompileOptionsWidget& operator=(const CompileOptionsWidget&) = delete;
        CompileOptionsWidget(CompileOptionsWidget&&) = delete;
        CompileOptionsWidget& operator=(CompileOptionsWidget&&) = delete;

        void rescan();
        void reload();

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;

    private:
        enum class Action : std::uint8_t {
            NameField,
            Save,
            Reload,
            EnableAll,
            DisableAll,
            Toggle,
        };

        struct Hit {
            Hit(Rect rect, Action action, CompileOptions::Kind kind = CompileOptions::Kind::System, std::string name = {})
                : rect(rect), action(action), kind(kind), name(std::move(name)) {}

            Rect rect;
            Action action;
            CompileOptions::Kind kind;
            std::string name;
        };

        CompileOptions _saved;
        CompileOptions _draft;
        std::vector<Hit> _hits;
        std::string _message;
        bool _messageIsError = false;
        bool _editing = false;
        int _scroll = 0;
        int _contentHeight = 0;
        int _viewHeight = 0;

        [[nodiscard]] bool _isDirty() const { return !(this->_draft == this->_saved); }
        void _save();
        void _onClick(Vector2 localMouse);
        bool _onKey(const Event& event);
        void _drawHeader(const IGraphic& graphic, int y);
        [[nodiscard]] int _drawSection(const IGraphic& graphic, CompileOptions::Kind kind, int y, int width);
};

} // namespace OrByte

#endif /* COMPILEOPTIONSWIDGET_HPP_ */
