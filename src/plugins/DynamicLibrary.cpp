/**
 * @file DynamicLibrary.cpp
 * @brief Dynamic Library
 *
 * @author SkyReox
 * @date 2026-10-12
**/

#include "plugins/DynamicLibrary.hpp"

OrByte::DynamicLibrary::DynamicLibrary(const std::filesystem::path& path) : _path(path)
{
#ifdef _WIN32
    this->_handle = LoadLibraryA(path.string().c_str());
    if (!this->_handle)
        throw LibraryLoadFailed(path.string(), "error code " + std::to_string(GetLastError()));
#else
    this->_handle = dlopen(path.c_str(), RTLD_NOW);
    if (!this->_handle)
        throw LibraryLoadFailed(path, dlerror());
#endif

    this->type = this->loadFunction<OrByte::DynamicLibrary::PluginType (*)()>("type");
}

OrByte::DynamicLibrary::~DynamicLibrary() noexcept
{
    if (!this->_handle)
        return;
#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(this->_handle));
#else
    dlclose(this->_handle);
#endif
}
