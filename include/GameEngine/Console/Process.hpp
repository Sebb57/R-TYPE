/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Process.hpp
*/

#pragma once
#ifndef PROCESS_HPP_
    #define PROCESS_HPP_

    #include <exception>
    #include <filesystem>
    #include <optional>
    #include <string>
    #include <utility>

namespace OrByte {

class ProcessException : public std::exception {
    public:
        explicit ProcessException(std::string msg) : _msg(std::move(msg)) {}
        [[nodiscard]] const char *what() const noexcept override { return this->_msg.c_str(); }

    private:
        std::string _msg;
};

class ProcessUnsupported : public ProcessException {
    public:
        ProcessUnsupported() : ProcessException("the console can't run programs on Windows") {}
};

class ProcessSpawnFailed : public ProcessException {
    public:
        explicit ProcessSpawnFailed(const std::string& reason) : ProcessException(reason + " failed") {}
};

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
