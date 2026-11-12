/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Console.cpp
*/

#include "GameEngine/Console/Console.hpp"
#include "GameEngine/ECS/WorldSerializer.hpp"
#include "constants.hpp"
#include <algorithm>
#include <exception>
#include <format>
#include <functional>
#include <optional>
#include <ranges>
#include <set>
#include <sstream>
#include <string_view>
#include <utility>

namespace {

#ifdef _WIN32
constexpr std::string_view SHELL_HELP = "anything else runs in cmd, like dir";
constexpr std::string_view CONFIGURE_FLAGS = " -DCMAKE_BUILD_TYPE=Release";
constexpr std::string_view BUILD_FLAGS = " --config Release --parallel";
#else
constexpr std::string_view SHELL_HELP = "anything else runs in sh, like ls";
constexpr std::string_view CONFIGURE_FLAGS;
constexpr std::string_view BUILD_FLAGS;
#endif

[[nodiscard]] std::filesystem::path gameWorldPath()
{
#ifdef _WIN32
    return std::filesystem::temp_directory_path() / std::filesystem::path(OrByte::constants::GAME_WORLD).filename();
#else
    return OrByte::constants::GAME_WORLD;
#endif
}

} // namespace

OrByte::Console::Console(SceneCollection& scenes, const ComponentPlugins& components)
    : _scenes(scenes), _components(components), _root(std::filesystem::current_path()), _cwd(_root)
{
    this->_print("Type 'help' to list the commands", LineKind::Info);
}

void OrByte::Console::execute(const std::string& input)
{
    std::istringstream stream(input);
    std::string name;
    std::string argument;

    stream >> name >> argument;
    this->_print(this->getPrompt() + input, LineKind::Command);
    if (name.empty())
        return;
    this->_history.push_back(input);

    const auto command = this->_commands.find(name);

    if (command != this->_commands.end())
        std::invoke(command->second.run, this, argument);
    else
        this->_run(input, this->_cwd);
}

void OrByte::Console::update()
{
    if (!this->_process)
        return;

    const std::optional<int> code = this->_process->exitCode();

    this->_printOutput(this->_process->read());
    if (!code)
        return;
    if (!this->_partial.empty())
        this->_printOutput("\n");
    if (this->_compiling)
        this->_print(*code == 0 ? "Compilation succeeded" : "Compilation failed", *code == 0 ? LineKind::Success : LineKind::Error);
    else if (*code != 0)
        this->_print(std::format("exited with code {}", *code), LineKind::Error);
    this->_process.reset();
    this->_compiling = false;
}

std::string OrByte::Console::getPrompt() const
{
    const std::filesystem::path relative = this->_cwd.lexically_relative(this->_root);

    if (relative.empty() || *relative.begin() == "..")
        return this->_cwd.string() + " $ ";
    if (relative == ".")
        return this->_root.filename().string() + " $ ";
    return (this->_root.filename() / relative).generic_string() + " $ ";
}

void OrByte::Console::_print(std::string text, LineKind kind)
{
    this->_lines.push_back({std::move(text), kind});
}

void OrByte::Console::_printOutput(const std::string& output)
{
    this->_partial += output;
    for (std::size_t end = this->_partial.find('\n'); end != std::string::npos; end = this->_partial.find('\n')) {
        std::string line = this->_partial.substr(0, end);

        std::ranges::replace(line, '\t', ' ');
        std::erase_if(line, [](char character) { return character < ' ' || character > '~'; });
        this->_print(std::move(line));
        this->_partial.erase(0, end + 1);
    }
}

void OrByte::Console::_help([[maybe_unused]] const std::string& argument)
{
    const std::size_t width = std::ranges::max(this->_commands | std::views::keys
        | std::views::transform([](const std::string& name) { return name.size(); })) + 2;

    for (const auto& [name, command] : this->_commands)
        this->_print(std::format("{:<{}}{}", name, width, command.help), LineKind::Info);
    this->_print(std::string(SHELL_HELP), LineKind::Info);
}

void OrByte::Console::_clear([[maybe_unused]] const std::string& argument)
{
    this->_lines.clear();
}

void OrByte::Console::_changeDirectory(const std::string& path)
{
    const std::filesystem::path target = path.empty() ? this->_root : this->_cwd / path;
    std::error_code error;

    if (!std::filesystem::is_directory(target, error)) {
        this->_print("cd: no such directory: " + path, LineKind::Error);
        return;
    }
    this->_cwd = std::filesystem::canonical(target, error);
}

void OrByte::Console::_compile([[maybe_unused]] const std::string& argument)
{
    if (this->_process) {
        this->_print("A command is already running", LineKind::Error);
        return;
    }
    if (this->_scenes.empty()) {
        this->_print("No scene to compile", LineKind::Error);
        return;
    }

    CompileOptions options = CompileOptions::load(this->_root / constants::COMPILE_OPTIONS);

    options.scan(this->_root / constants::PLUGINS_DIR);
    if (const std::optional<std::string> error = options.checkBinaryName(this->_root)) {
        this->_print(*error + ", change it in Compile Options", LineKind::Error);
        return;
    }
    std::filesystem::path worldPath;

    try {
        World& world = this->_scenes.currentWorld();

        worldPath = gameWorldPath();
        world.ensureCamera();
        serializeWorld(world, worldPath);
    } catch (const std::exception& error) {
        this->_print(std::string("Cannot export world: ") + error.what(), LineKind::Error);
        return;
    }

    const std::vector<std::string> systems = options.getEnabled(CompileOptions::Kind::System);
    const std::vector<std::string> components = options.getEnabled(CompileOptions::Kind::Component);

    this->_print(std::format("Compiling {} with {}/{} systems and {}/{} components", options.getBinaryName(),
        systems.size(), options.getPlugins(CompileOptions::Kind::System).size(),
        components.size(), options.getPlugins(CompileOptions::Kind::Component).size()), LineKind::Info);
    if (!options.getPlugins(CompileOptions::Kind::System).empty() && !std::ranges::contains(systems, "Display"))
        this->_print("Display is not compiled: the game won't draw anything", LineKind::Info);
    this->_run(std::format("cmake -S cmake/GameEngine -B build-solo \"-DORBYTE_GAME_WORLD={}\" {}{} && cmake --build build-solo{}",
        worldPath.generic_string(), options.cmakeArguments(), CONFIGURE_FLAGS, BUILD_FLAGS), this->_root);
    this->_compiling = this->_process != nullptr;
}

void OrByte::Console::_run(const std::string& command, const std::filesystem::path& cwd)
{
    if (this->_process) {
        this->_print("A command is already running", LineKind::Error);
        return;
    }
    try {
        this->_process = std::make_unique<Process>(command, cwd);
    } catch (const std::exception& error) {
        this->_print(error.what(), LineKind::Error);
        return;
    }
    if (this->_onStart)
        this->_onStart();
}
