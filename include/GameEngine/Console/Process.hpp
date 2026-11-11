/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Process.hpp
*/

#pragma once
#ifndef PROCESS_HPP_
    #define PROCESS_HPP_

    #include <filesystem>
    #include <optional>
    #include <string>

namespace OrByte {

class Process {
    public:
        Process(const std::string& command, const std::filesystem::path& cwd);
        ~Process();

        Process(const Process&) = delete;
        Process& operator=(const Process&) = delete;
        Process(Process&&) = delete;
        Process& operator=(Process&&) = delete;

        [[nodiscard]] std::string read();
        [[nodiscard]] std::optional<int> exitCode();

    private:
        int _pid = -1;
        int _output = -1;
        std::optional<int> _code;
};

} // namespace OrByte

#endif /* PROCESS_HPP_ */
