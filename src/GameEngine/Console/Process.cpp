/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Process.cpp
*/

#include "GameEngine/Console/Process.hpp"
#include <array>
#include <stdexcept>

#ifndef _WIN32
    #include <fcntl.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#ifdef _WIN32

OrByte::Process::Process([[maybe_unused]] const std::string& command, [[maybe_unused]] const std::filesystem::path& cwd)
{
    throw std::runtime_error("the console can't run programs on Windows");
}

OrByte::Process::~Process() = default;

std::string OrByte::Process::read()
{
    return {};
}

std::optional<int> OrByte::Process::exitCode()
{
    return this->_code;
}

#else

OrByte::Process::Process(const std::string& command, const std::filesystem::path& cwd)
{
    const std::string dir = cwd.string();
    std::array<int, 2> fds{};

    if (::pipe(fds.data()) < 0)
        throw std::runtime_error("pipe failed");
    this->_pid = ::fork();
    if (this->_pid < 0) {
        ::close(fds[0]);
        ::close(fds[1]);
        throw std::runtime_error("fork failed");
    }
    if (this->_pid == 0) {
        const int devNull = ::open("/dev/null", O_RDONLY);

        ::dup2(devNull, STDIN_FILENO);
        ::dup2(fds[1], STDOUT_FILENO);
        ::dup2(fds[1], STDERR_FILENO);
        ::close(fds[0]);
        ::close(fds[1]);
        if (::chdir(dir.c_str()) == 0)
            ::execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        ::_exit(127);
    }
    ::close(fds[1]);
    this->_output = fds[0];
    ::fcntl(this->_output, F_SETFL, O_NONBLOCK);
}

OrByte::Process::~Process()
{
    ::close(this->_output);
    if (this->_pid > 0)
        ::waitpid(this->_pid, nullptr, WNOHANG);
}

std::string OrByte::Process::read()
{
    std::string output;
    std::array<char, 4096> buffer{};

    for (ssize_t count = ::read(this->_output, buffer.data(), buffer.size()); count > 0;
         count = ::read(this->_output, buffer.data(), buffer.size()))
        output.append(buffer.data(), static_cast<std::size_t>(count));
    return output;
}

std::optional<int> OrByte::Process::exitCode()
{
    int status = 0;

    if (this->_pid > 0 && ::waitpid(this->_pid, &status, WNOHANG) == this->_pid) {
        this->_pid = -1;
        this->_code = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
    }
    return this->_code;
}

#endif
