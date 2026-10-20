/**
 * @file DynamicLibrary.hpp
 * @brief Dynamic Library Loader
 *
 * @author SkyReox
 * @date 2026-10-12
**/

#pragma once
#ifndef DYNAMICLIBRARY_HPP_
    #define DYNAMICLIBRARY_HPP_

#include <dlfcn.h>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

namespace OrByte {

class DynamicLibrary {
    void* _handle = nullptr;
    std::filesystem::path _path;

    public:
        enum PluginType {
            GRAPHICAL_LIB,
        };

        DynamicLibrary(const std::filesystem::path& path);
        ~DynamicLibrary() noexcept;

        PluginType (*type)() = nullptr;

        std::string getPath() const noexcept { return this->_path; }
        template <typename T>
        T loadFunction(std::string symbol)
        {
            dlerror();
            T func = reinterpret_cast<T>(dlsym(this->_handle, symbol.c_str()));
            const char *error = dlerror();

            if (error != nullptr || !func) {
                std::cerr << "ERROR: " << error << std::endl;
                throw FunctionLoadFailed(symbol);
            }
            return func;
        }

        class DynamicLibraryException : public std::exception {
            protected:
                std::string _msg;

            public:
                DynamicLibraryException(const std::string& msg) : _msg(msg) {}
                virtual const char* what() const noexcept override { return this->_msg.c_str(); }
        };

        class LibraryLoadFailed : public DynamicLibraryException {
            public:
                LibraryLoadFailed(const std::string& path, const std::string& reason = "")
                    : DynamicLibraryException(
                        "Unable to load dynamic library: " + path
                        + (reason.empty() ? "" : " (" + reason + ")")
                    ) {}
        };

        class FunctionLoadFailed : public DynamicLibraryException {
            public:
                FunctionLoadFailed(const std::string& funcName)
                    : DynamicLibraryException("Unable to load function: " + funcName) {}
        };

        class NotGraphicalLibrary : public DynamicLibraryException {
            public:
                NotGraphicalLibrary(const std::string& path)
                    : DynamicLibraryException("Error: '" + path + "' not a graphical library") {}
        };
};

}

#endif /* DYNAMICLIBRARY_HPP_ */
