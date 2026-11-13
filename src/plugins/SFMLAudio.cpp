/**
 * @file SFMLAudio.cpp
 * @brief IAudio implementation with SFML
 *
 * @author SkyReox
 * @date 2026-10-12
**/

#include "plugins/SFMLAudio.hpp"
#include "plugins/DynamicLibrary.hpp"
#include <algorithm>

OrByte::SFMLAudio::~SFMLAudio()
{
    for (auto& [channel, music] : this->_musics)
        music->stop();
}

sf::Music *OrByte::SFMLAudio::_get(int channel) const
{
    const auto it = this->_musics.find(channel);
    return it != this->_musics.end() ? it->second.get() : nullptr;
}

bool OrByte::SFMLAudio::load(int channel, const std::filesystem::path& path)
{
    auto music = std::make_unique<sf::Music>();

    this->unload(channel);
    if (!music->openFromFile(path.string()))
        return false;
    this->_musics[channel] = std::move(music);
    return true;
}

void OrByte::SFMLAudio::unload(int channel)
{
    if (sf::Music *music = this->_get(channel))
        music->stop();
    this->_musics.erase(channel);
}

bool OrByte::SFMLAudio::isLoaded(int channel) const
{
    return this->_get(channel) != nullptr;
}

void OrByte::SFMLAudio::play(int channel)
{
    if (sf::Music *music = this->_get(channel))
        music->play();
}

void OrByte::SFMLAudio::pause(int channel)
{
    if (sf::Music *music = this->_get(channel))
        music->pause();
}

void OrByte::SFMLAudio::stop(int channel)
{
    if (sf::Music *music = this->_get(channel))
        music->stop();
}

OrByte::AudioStatus OrByte::SFMLAudio::getStatus(int channel) const
{
    const sf::Music *music = this->_get(channel);

    if (music == nullptr)
        return AudioStatus::Stopped;
    switch (music->getStatus()) {
        case sf::SoundSource::Status::Playing:
            return AudioStatus::Playing;
        case sf::SoundSource::Status::Paused:
            return AudioStatus::Paused;
        default:
            return AudioStatus::Stopped;
    }
}

void OrByte::SFMLAudio::setVolume(int channel, float volume)
{
    if (sf::Music *music = this->_get(channel))
        music->setVolume(std::clamp(volume, 0.f, 100.f));
}

void OrByte::SFMLAudio::setPitch(int channel, float pitch)
{
    if (sf::Music *music = this->_get(channel))
        music->setPitch(std::max(pitch, 0.01f));
}

void OrByte::SFMLAudio::setLoop(int channel, bool loop)
{
    if (sf::Music *music = this->_get(channel))
        music->setLooping(loop);
}

void OrByte::SFMLAudio::setOffset(int channel, float seconds)
{
    if (sf::Music *music = this->_get(channel))
        music->setPlayingOffset(sf::seconds(std::max(seconds, 0.f)));
}

float OrByte::SFMLAudio::getOffset(int channel) const
{
    const sf::Music *music = this->_get(channel);
    return music != nullptr ? music->getPlayingOffset().asSeconds() : 0.f;
}

float OrByte::SFMLAudio::getDuration(int channel) const
{
    const sf::Music *music = this->_get(channel);
    return music != nullptr ? music->getDuration().asSeconds() : 0.f;
}

void OrByte::SFMLAudio::setMasterVolume(float volume)
{
    sf::Listener::setGlobalVolume(std::clamp(volume, 0.f, 100.f));
}

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::AUDIO_LIB;
    }

    OrByte::IAudio *factory()
    {
        return new OrByte::SFMLAudio();
    }
}
