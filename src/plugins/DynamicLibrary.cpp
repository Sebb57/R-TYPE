/**
 * @file DynamicLibrary.cpp
 * @brief Dynamic Library
 *
 * @author SkyReox
 * @date 2026-10-12
**/

#include "plugins/DynamicLibrary.hpp"

DynamicLibrary::DynamicLibrary(const std::string& path) : _path(path)
{
#ifdef _WIN32
    this->_handle = LoadLibraryA(path.c_str());
    if (!this->_handle)
        throw LibraryLoadFailed(path, "error code " + std::to_string(GetLastError()));
#else
    this->_handle = dlopen(path.c_str(), RTLD_NOW);
    if (!this->_handle)
        throw LibraryLoadFailed(path, dlerror());
#endif

    try {
        this->type = this->loadFunction<std::size_t (*)()>("type");
    } catch (const DynamicLibraryException& e) {
        std::cout << e.what() << std::endl;
        return;
    }
}

DynamicLibrary::~DynamicLibrary() noexcept
{
    if (!this->_handle)
        return;
#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(this->_handle));
#else
    dlclose(this->_handle);
#endif
}
