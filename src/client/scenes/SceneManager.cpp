/**
 * @file SceneManager.cpp
 * @brief scene registry and switching implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "client/scenes/SceneManager.hpp"
#include <iostream>
#include <utility>

void client::SceneManager::add(const std::string& name, Factory factory)
{
    this->_factories.insert_or_assign(name, std::move(factory));
}

void client::SceneManager::switchTo(const std::string& name)
{
    this->_next = name;
}

void client::SceneManager::apply()
{
    if (!this->_next)
        return;
    const std::string name = *this->_next;
    this->_next.reset();
    const auto factory = this->_factories.find(name);
    if (factory == this->_factories.end()) {
        std::cerr << "[Client] unknown scene " << name << std::endl;
        return;
    }
    this->_current.reset();
    this->_current = factory->second();
}

void client::SceneManager::onEvent(const OrByte::Event& event)
{
    if (this->_current)
        this->_current->onEvent(event);
}

void client::SceneManager::update(double deltaTime)
{
    if (this->_current)
        this->_current->update(deltaTime);
}

void client::SceneManager::render()
{
    if (this->_current)
        this->_current->render();
}
