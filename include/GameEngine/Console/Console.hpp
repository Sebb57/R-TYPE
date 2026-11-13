/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Console.hpp
*/

#pragma once
#ifndef CONSOLE_HPP_
    #define CONSOLE_HPP_

    #include "GameEngine/Console/CompileOptions.hpp"
    #include "GameEngine/Console/Process.hpp"
    #include "GameEngine/ECS/ComponentPlugins.hpp"
    #include "GameEngine/ECS/SceneCollection.hpp"
    #include <cstdint>
    #include <filesystem>
    #include <functional>
    #include <map>
    #include <memory>
    #include <string>
    #include <vector>

namespace OrByte {

class Console {
    public:
        enum class LineKind : std::uint8_t {
            Output,
            Command,
            Info,
            Success,
            Error,
        };

        struct Line {
            std::string text;
            LineKind kind = LineKind::Output;
        };

        Console(SceneCollection& scenes, const ComponentPlugins& components);

        void execute(const std::string& input);
        void update();

        [[nodiscard]] const std::vector<Line>& getLines() const noexcept { return this->_lines; }
        [[nodiscard]] const std::vector<std::string>& getHistory() const noexcept { return this->_history; }
        [[nodiscard]] std::string getPrompt() const;
        void setOnStart(std::function<void()> callback) { this->_onStart = std::move(callback); }

    private:
        struct Command {
            void (Console::*run)(const std::string& argument);
            std::string help;
        };

        const std::map<std::string, Command> _commands{
            {"cd", {&Console::_changeDirectory, "change the console directory, back to the project without a path"}},
            {"clear", {&Console::_clear, "empty the console"}},
            {"compile", {&Console::_compile, "export the world and build the game with the saved compile options"}},
            {"help", {&Console::_help, "list the commands"}},
        };

        SceneCollection& _scenes;
        const ComponentPlugins& _components;
        std::filesystem::path _root;
        std::filesystem::path _cwd;
        std::vector<Line> _lines;
        std::vector<std::string> _history;
        std::function<void()> _onStart;
        std::unique_ptr<Process> _process;
        std::string _partial;
        bool _compiling = false;

        void _changeDirectory(const std::string& path);
        void _clear(const std::string& argument);
        void _compile(const std::string& argument);
        void _help(const std::string& argument);

        void _print(std::string text, LineKind kind = LineKind::Output);
        void _printOutput(const std::string& output);
        void _run(const std::string& command, const std::filesystem::path& cwd);
};

} // namespace OrByte

#endif /* CONSOLE_HPP_ */
