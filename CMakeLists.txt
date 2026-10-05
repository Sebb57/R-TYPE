cmake_minimum_required(VERSION 3.20)

project(RType VERSION 1.0 DESCRIPTION "R-Type multiplayer game" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include(FetchContent)
find_package(Threads REQUIRED)

FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-30-2
    GIT_SHALLOW TRUE
)

FetchContent_Declare(bitsery
    GIT_REPOSITORY https://github.com/fraillt/bitsery.git
    GIT_TAG v5.2.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(asio)
FetchContent_GetProperties(bitsery)
if(NOT bitsery_POPULATED)
    FetchContent_Populate(bitsery)
endif()

add_library(bitsery INTERFACE)
target_include_directories(bitsery SYSTEM INTERFACE "${bitsery_SOURCE_DIR}/include")

add_library(rtype_asio INTERFACE)
target_include_directories(rtype_asio SYSTEM INTERFACE "${asio_SOURCE_DIR}/asio/include")
target_compile_definitions(rtype_asio INTERFACE ASIO_STANDALONE)
target_link_libraries(rtype_asio INTERFACE Threads::Threads)

add_library(rtype-protocol INTERFACE)
target_include_directories(rtype-protocol INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(rtype-protocol INTERFACE rtype_asio bitsery)
target_compile_features(rtype-protocol INTERFACE cxx_std_20)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GameEngine/*.cpp")
add_library(rtype_engine STATIC ${ENGINE_SOURCES})
target_include_directories(rtype_engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/GameEngine/ECS/Managers"
)
target_compile_features(rtype_engine PUBLIC cxx_std_20)

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cxx")
file(GLOB_RECURSE SERVER_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/server/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/server/*.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/server/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/server/*.hxx")

# Make every directory below include/server available for both flat includes
# (e.g. "ServerConfig.hpp") and tree-qualified includes
# (e.g. "common/ServerConfig.hpp"). CONFIGURE_DEPENDS makes CMake re-run when
# files are added in a new nested folder.
set(SERVER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
foreach(header IN LISTS SERVER_HEADERS)
    get_filename_component(header_dir "${header}" DIRECTORY)
    list(APPEND SERVER_INCLUDE_DIRS "${header_dir}")
endforeach()
list(REMOVE_DUPLICATES SERVER_INCLUDE_DIRS)

add_executable(server ${SERVER_SOURCES})
target_include_directories(server PRIVATE ${SERVER_INCLUDE_DIRS})
target_link_libraries(server PRIVATE rtype_engine rtype-protocol)
target_compile_features(server PRIVATE cxx_std_20)

option(RTYPE_BUILD_CLIENT "Build the SFML client" ON)
if(RTYPE_BUILD_CLIENT)
    set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(sfml)

    add_executable(r-type_client src/main.cpp)
    target_include_directories(r-type_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(r-type_client PRIVATE rtype_engine sfml-graphics sfml-window sfml-audio sfml-system)
    target_compile_features(r-type_client PRIVATE cxx_std_20)
endif()

foreach(target IN ITEMS rtype_engine server)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
if(TARGET r-type_client)
    if(MSVC)
        target_compile_options(r-type_client PRIVATE /W4 /permissive-)
    else()
        target_compile_options(r-type_client PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
