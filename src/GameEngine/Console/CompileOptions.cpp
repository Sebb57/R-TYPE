/**
 * @file CompileOptions.cpp
 * @brief compile options implementation
 *
 * @author Seb
 * @date 2026-11-11
**/

#include "GameEngine/Console/CompileOptions.hpp"
#include <algorithm>
#include <array>
#include <format>
#include <fstream>
#include <ranges>
#include <system_error>

namespace {

#ifdef _WIN32
constexpr std::string_view PLUGIN_EXTENSION = ".dll";
constexpr std::string_view EXECUTABLE_EXTENSION = ".exe";
#else
constexpr std::string_view PLUGIN_EXTENSION = ".so";
constexpr std::string_view EXECUTABLE_EXTENSION;
#endif

constexpr std::string_view SYSTEM_SUFFIX = "System";
constexpr std::string_view BINARY_KEY = "binary";
constexpr std::string_view DISABLED_SYSTEMS_KEY = "disabled_systems";
constexpr std::string_view DISABLED_COMPONENTS_KEY = "disabled_components";

constexpr std::array<std::string_view, 3> RESERVED_BINARIES{"r-type_server", "r-type_client", "r-type_game-engine"};

[[nodiscard]] std::string trim(std::string_view text)
{
    const auto first = text.find_first_not_of(" \t\r");

    if (first == std::string_view::npos)
        return {};
    return std::string(text.substr(first, text.find_last_not_of(" \t\r") - first + 1));
}

[[nodiscard]] std::set<std::string> parseNames(std::string_view list)
{
    std::set<std::string> names;

    for (const auto part : std::views::split(list, ',')) {
        std::string name = trim(std::string_view(part.begin(), part.end()));

        if (OrByte::isPluginName(name))
            names.insert(std::move(name));
    }
    return names;
}

[[nodiscard]] std::string joinNames(const auto& names, char separator)
{
    std::string joined;

    for (const std::string& name : names) {
        if (!joined.empty())
            joined.push_back(separator);
        joined += name;
    }
    return joined;
}

[[nodiscard]] std::vector<std::string> listPlugins(const std::filesystem::path& directory, std::string_view suffix)
{
    std::vector<std::string> names;
    std::error_code error;

    for (const auto& entry : std::filesystem::directory_iterator(directory, error)) {
        if (!entry.is_regular_file(error) || entry.path().extension() != PLUGIN_EXTENSION)
            continue;
        std::string name = entry.path().stem().string();

        if (!suffix.empty() && name.ends_with(suffix) && name.size() > suffix.size())
            name.erase(name.size() - suffix.size());
        if (OrByte::isPluginName(name))
            names.push_back(std::move(name));
    }
    std::ranges::sort(names);
    return names;
}

[[nodiscard]] bool isBinaryChar(char character)
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9') || character == '_' || character == '-' || character == '.';
}

[[nodiscard]] std::string cmakeList(const OrByte::CompileOptions& options, OrByte::CompileOptions::Kind kind)
{
    if (options.getPlugins(kind).empty())
        return "ALL";
    return joinNames(options.getEnabled(kind), ';');
}

} // namespace

bool OrByte::isPluginName(std::string_view name) noexcept
{
    return !name.empty() && std::ranges::all_of(name, [](char character) {
        return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')
            || (character >= '0' && character <= '9') || character == '_';
    });
}

OrByte::CompileOptions OrByte::CompileOptions::load(const std::filesystem::path& path)
{
    CompileOptions options;
    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line)) {
        const std::string content = trim(line);
        const std::size_t equal = content.find('=');

        if (content.empty() || content.starts_with('#') || equal == std::string::npos)
            continue;
        const std::string key = trim(std::string_view(content).substr(0, equal));
        const std::string value = trim(std::string_view(content).substr(equal + 1));

        if (key == BINARY_KEY && !value.empty())
            options._binaryName = value;
        else if (key == DISABLED_SYSTEMS_KEY)
            options._disabledSystems = parseNames(value);
        else if (key == DISABLED_COMPONENTS_KEY)
            options._disabledComponents = parseNames(value);
    }
    return options;
}

void OrByte::CompileOptions::save(const std::filesystem::path& path) const
{
    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path());

    std::ofstream file(path, std::ios::trunc);

    if (!file.is_open())
        throw CompileOptionsOpenFailed(path);
    file << "# OrByte compile options, edited from the Compile Options tab" << std::endl
         << BINARY_KEY << '=' << this->_binaryName << std::endl
         << DISABLED_SYSTEMS_KEY << '=' << joinNames(this->_disabledSystems, ',') << std::endl
         << DISABLED_COMPONENTS_KEY << '=' << joinNames(this->_disabledComponents, ',') << std::endl;
    if (!file)
        throw CompileOptionsWriteFailed(path);
}

void OrByte::CompileOptions::scan(const std::filesystem::path& pluginsDirectory)
{
    this->_systems = listPlugins(pluginsDirectory / "systems", SYSTEM_SUFFIX);
    this->_components = listPlugins(pluginsDirectory / "components", "");
}

const std::vector<std::string>& OrByte::CompileOptions::getPlugins(Kind kind) const noexcept
{
    return kind == Kind::System ? this->_systems : this->_components;
}

std::set<std::string>& OrByte::CompileOptions::_disabled(Kind kind) noexcept
{
    return kind == Kind::System ? this->_disabledSystems : this->_disabledComponents;
}

const std::set<std::string>& OrByte::CompileOptions::_disabled(Kind kind) const noexcept
{
    return kind == Kind::System ? this->_disabledSystems : this->_disabledComponents;
}

bool OrByte::CompileOptions::isEnabled(Kind kind, const std::string& name) const
{
    return !this->_disabled(kind).contains(name);
}

void OrByte::CompileOptions::setEnabled(Kind kind, const std::string& name, bool enabled)
{
    if (enabled)
        this->_disabled(kind).erase(name);
    else if (isPluginName(name))
        this->_disabled(kind).insert(name);
}

void OrByte::CompileOptions::setAllEnabled(Kind kind, bool enabled)
{
    for (const std::string& name : this->getPlugins(kind))
        this->setEnabled(kind, name, enabled);
}

std::vector<std::string> OrByte::CompileOptions::getEnabled(Kind kind) const
{
    std::vector<std::string> enabled;

    for (const std::string& name : this->getPlugins(kind))
        if (this->isEnabled(kind, name))
            enabled.push_back(name);
    return enabled;
}

std::optional<std::string> OrByte::CompileOptions::checkBinaryName(const std::filesystem::path& root) const
{
    const std::string& name = this->_binaryName;

    if (name.empty())
        return "The binary name is empty";
    if (name.size() > BINARY_MAX_LENGTH)
        return std::format("The binary name is longer than {} characters", BINARY_MAX_LENGTH);
    if (name.front() == '-' || name.front() == '.')
        return "The binary name can't start with '-' or '.'";
    if (!std::ranges::all_of(name, isBinaryChar))
        return "The binary name only takes letters, digits, '_', '-' and '.'";
    if (std::ranges::contains(RESERVED_BINARIES, name))
        return name + " is already a binary of the project";

    const std::filesystem::path target = root / (name + std::string(EXECUTABLE_EXTENSION));
    std::error_code error;
    const std::filesystem::file_status status = std::filesystem::status(target, error);

    if (!std::filesystem::exists(status))
        return std::nullopt;
    if (!std::filesystem::is_regular_file(status))
        return target.filename().string() + " already exists and is not a file";
#ifndef _WIN32
    if ((status.permissions() & std::filesystem::perms::owner_exec) == std::filesystem::perms::none)
        return "Compiling would overwrite " + target.filename().string();
#endif
    return std::nullopt;
}

std::string OrByte::CompileOptions::cmakeArguments() const
{
    return std::format("-DORBYTE_GAME_NAME={} \"-DORBYTE_SYSTEMS={}\" \"-DORBYTE_COMPONENTS={}\"",
        this->_binaryName, cmakeList(*this, Kind::System), cmakeList(*this, Kind::Component));
}

bool OrByte::CompileOptions::operator==(const CompileOptions& other) const
{
    return this->_binaryName == other._binaryName
        && this->_disabledSystems == other._disabledSystems
        && this->_disabledComponents == other._disabledComponents;
}
