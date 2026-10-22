/**
 * @file ServerScene.cpp
 * @brief server choice menu implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "client/scenes/ServerScene.hpp"
#include <cctype>
#include "client/network/NetworkClient.hpp"
#include "client/scenes/RoomListScene.hpp"
#include "client/scenes/SceneManager.hpp"

namespace {

constexpr const char *LocalHost = "127.0.0.1";
constexpr std::size_t MaxAddressLength = 15;
constexpr std::size_t MaxNameLength = 16;

bool isAddressCharacter(char character)
{
    return std::isdigit(static_cast<unsigned char>(character)) != 0 || character == '.';
}

bool isNameCharacter(char character)
{
    return std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_' || character == '-';
}

} // namespace

client::ServerScene::ServerScene(SceneContext& context)
    : AMenuScene(context)
    , _address(context.config.host == LocalHost ? "" : context.config.host)
    , _username(context.config.username)
{
    this->_subtitle = "CHOOSE A SERVER";
    this->addButton("Local server", [this] { this->connect(LocalHost); }, [this] { return !this->_username.empty(); }, LocalHost);
    this->addField("Other server", this->_address, MaxAddressLength, isAddressCharacter, [this] { this->connect(this->_address); });
    this->addField("Player name", this->_username, MaxNameLength, isNameCharacter);
    this->addButton("Quit", [this] { this->_context.graphic.close(); });
    this->bindAction(Action::Back, [this](bool pressed) {
        if (pressed)
            this->_context.graphic.close();
    });
}

void client::ServerScene::update(double deltaTime)
{
    AMenuScene::update(deltaTime);
    if (this->_view.connected)
        this->_context.scenes.switchTo(RoomListScene::Name);
}

void client::ServerScene::connect(const std::string& host)
{
    if (host.empty() || this->_username.empty())
        return;
    this->_context.network.connect(host, this->_context.config.tcpPort, this->_username);
}
