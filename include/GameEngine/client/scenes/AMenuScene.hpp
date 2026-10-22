/**
 * @file AMenuScene.hpp
 * @brief base menu scene, a navigable list of buttons and text fields
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_SCENES_AMENUSCENE_HPP
#define GAMEENGINE_CLIENT_SCENES_AMENUSCENE_HPP

#include <cstddef>
#include <functional>
#include <string>
#include <vector>
#include "GameEngine/client/network/ClientView.hpp"
#include "GameEngine/client/scenes/AScene.hpp"

namespace client {

class AMenuScene : public AScene {
    public:
        explicit AMenuScene(SceneContext& context);

        void onEvent(const OrByte::Event& event) override;
        void update(double deltaTime) override;
        void render() override;

    protected:
        using Callback = std::function<void()>;
        using Condition = std::function<bool()>;
        using Filter = std::function<bool(char)>;

        void addButton(std::string label, Callback onSelect, Condition enabled = nullptr, std::string detail = "");
        void addField(std::string label, std::string& value, std::size_t maxLength, Filter accepts, Callback onSelect = nullptr);
        void clearEntries();

        ClientView _view;
        std::string _subtitle;
        float _top = 260.0f;

    private:
        struct Entry {
            std::string label;
            std::string detail;
            Callback onSelect;
            Condition enabled;
            std::string *field = nullptr;
            std::size_t maxLength = 0;
            Filter accepts;
        };

        void move(int step);
        void activate();
        void drawEntry(const Entry& entry, bool selected, float y) const;
        [[nodiscard]] std::string hints() const;

        std::vector<Entry> _entries;
        std::size_t _selected = 0;
};

} // namespace client

#endif // GAMEENGINE_CLIENT_SCENES_AMENUSCENE_HPP
