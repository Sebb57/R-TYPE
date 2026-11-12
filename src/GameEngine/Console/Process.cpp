/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Process.cpp
*/

#include "GameEngine/Console/Process.hpp"
#include <array>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <algorithm>
#else
    #include <fcntl.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#ifdef _WIN32

namespace {

constexpr DWORD PIPE_SIZE = 64 * 1024;

[[nodiscard]] std::string commandInterpreter()
{
    std::array<char, MAX_PATH> system{};
    const UINT length = ::GetSystemDirectoryA(system.data(), static_cast<UINT>(system.size()));

    if (length == 0 || length >= system.size())
        throw OrByte::ProcessSpawnFailed("GetSystemDirectory");
    return std::string(system.data(), length) + "\\cmd.exe";
}

} // namespace

OrByte::Process::Process(const std::string& command, const std::filesystem::path& cwd)
{
    const std::string interpreter = commandInterpreter();
    const std::string dir = cwd.string();
    std::string line = "cmd.exe /d /s /c \"" + command + "\"";
    SECURITY_ATTRIBUTES inherit{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE output = nullptr;
    HANDLE write = nullptr;

    if (!::CreatePipe(&output, &write, &inherit, PIPE_SIZE))
        throw ProcessSpawnFailed("CreatePipe");
    ::SetHandleInformation(output, HANDLE_FLAG_INHERIT, 0);

    const HANDLE input = ::CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &inherit,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    STARTUPINFOA startup{};
    PROCESS_INFORMATION info{};

    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input;
    startup.hStdOutput = write;
    startup.hStdError = write;

    const BOOL created = ::CreateProcessA(interpreter.c_str(), line.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, dir.c_str(), &startup, &info);

    if (input != INVALID_HANDLE_VALUE)
        ::CloseHandle(input);
    ::CloseHandle(write);
    if (!created) {
        ::CloseHandle(output);
        throw ProcessSpawnFailed("CreateProcess");
    }
    ::CloseHandle(info.hThread);
    this->_handle = info.hProcess;
    this->_output = output;
}

OrByte::Process::~Process()
{
    if (this->_output != nullptr)
        ::CloseHandle(this->_output);
    if (this->_handle != nullptr)
        ::CloseHandle(this->_handle);
}

std::string OrByte::Process::read()
{
    std::string output;
    std::array<char, 4096> buffer{};
    DWORD available = 0;

    while (::PeekNamedPipe(this->_output, nullptr, 0, nullptr, &available, nullptr) && available > 0) {
        DWORD count = 0;

        if (!::ReadFile(this->_output, buffer.data(), std::min<DWORD>(available, buffer.size()), &count, nullptr)
            || count == 0)
            break;
        output.append(buffer.data(), count);
    }
    return output;
}

std::optional<int> OrByte::Process::exitCode()
{
    DWORD code = 0;

    if (this->_handle != nullptr && ::WaitForSingleObject(this->_handle, 0) == WAIT_OBJECT_0
        && ::GetExitCodeProcess(this->_handle, &code)) {
        ::CloseHandle(this->_handle);
        this->_handle = nullptr;
        this->_code = static_cast<int>(code);
    }
    return this->_code;
}

#else

OrByte::Process::Process(const std::string& command, const std::filesystem::path& cwd)
{
    const std::string dir = cwd.string();
    std::array<int, 2> fds{};

    if (::pipe(fds.data()) < 0)
        throw ProcessSpawnFailed("pipe");
    this->_pid = ::fork();
    if (this->_pid < 0) {
        ::close(fds[0]);
        ::close(fds[1]);
        throw ProcessSpawnFailed("fork");
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
