/**
 * @file DynamicLibrary.cpp
 * @brief Dynamic Library
 *
 * @author SkyReox
 * @date 2026-10-12
**/

#include "plugins/DynamicLibrary.hpp"
#include <dlfcn.h>

OrByte::DynamicLibrary::DynamicLibrary(const std::filesystem::path& path) : _path(path)
{
    this->_handle = dlopen(path.c_str(), RTLD_NOW);
    if (!this->_handle)
        throw LibraryLoadFailed(path, dlerror());

    this->type = this->loadFunction<OrByte::DynamicLibrary::PluginType (*)()>("type");
}

OrByte::DynamicLibrary::~DynamicLibrary() noexcept
{
    dlclose(this->_handle);
}
