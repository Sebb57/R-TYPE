# Standalone build of the game produced by the engine:
#   cmake -S cmake/GameEngine -B build-solo && cmake --build build-solo
# (the engine builds in build-solo on Linux and build-solo-windows on Windows)
#
# The engine's "compile" command (Compile Options tab) sets:
#   ORBYTE_GAME_NAME    name of the game binary, written to the repository root
#   ORBYTE_SYSTEMS      ;-list of systems to compile (Control, Display, ...), ALL for every one
#   ORBYTE_COMPONENTS   ;-list of components to compile (Transform, Sprite, ...), ALL for every one
#   ORBYTE_METRICS      ;-list of metrics to compile (Lagometer, ...), each one defines ORBYTE_METRIC_<NAME>
# The game only loads the plugins it was compiled with.
cmake_minimum_required(VERSION 3.23)

project(RTypeSolo VERSION 1.0 DESCRIPTION "R-Type solo game engine" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

get_filename_component(RTYPE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

# ---------------------------------------------------------------------------
# Compile options
# ---------------------------------------------------------------------------
set(ORBYTE_GAME_NAME "OrByte-gameClient" CACHE STRING "Name of the game binary")
set(ORBYTE_SYSTEMS "ALL" CACHE STRING "Systems compiled into the game, ALL for every one")
set(ORBYTE_COMPONENTS "ALL" CACHE STRING "Components compiled into the game, ALL for every one")
set(ORBYTE_METRICS "" CACHE STRING "Metrics compiled into the game")

if(NOT ORBYTE_GAME_NAME MATCHES "^[A-Za-z0-9_][A-Za-z0-9_.-]*$")
    message(FATAL_ERROR "Invalid ORBYTE_GAME_NAME '${ORBYTE_GAME_NAME}': use letters, digits, '_', '-' and '.'")
endif()
# Windows file names ignore case, R-TYPE_SERVER.exe would overwrite r-type_server.exe
string(TOLOWER "${ORBYTE_GAME_NAME}" ORBYTE_GAME_NAME_LOWER)
if(ORBYTE_GAME_NAME_LOWER MATCHES "^(r-type_server|r-type_client|r-type_game-engine)$")
    message(FATAL_ERROR "ORBYTE_GAME_NAME '${ORBYTE_GAME_NAME}' would overwrite a binary of the project")
endif()

# rtype_select_plugins(<out> <requested> <available> <kind>): keeps the requested plugins that have a source
function(rtype_select_plugins out requested available kind)
    if(requested STREQUAL "ALL")
        set(${out} ${available} PARENT_SCOPE)
        return()
    endif()
    set(selected)
    foreach(name IN LISTS requested)
        if(name IN_LIST available)
            list(APPEND selected ${name})
        else()
            message(WARNING "No source for the ${kind} '${name}', it is not compiled")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES selected)
    set(${out} ${selected} PARENT_SCOPE)
endfunction()

file(GLOB AVAILABLE_SYSTEMS RELATIVE "${RTYPE_ROOT}/src/GameEngine/Systems" CONFIGURE_DEPENDS
    "${RTYPE_ROOT}/src/GameEngine/Systems/*System.cpp")
list(TRANSFORM AVAILABLE_SYSTEMS REPLACE "System\\.cpp$" "")
file(GLOB AVAILABLE_COMPONENTS RELATIVE "${RTYPE_ROOT}/src/GameEngine/Components" CONFIGURE_DEPENDS
    "${RTYPE_ROOT}/src/GameEngine/Components/*.cpp")
list(TRANSFORM AVAILABLE_COMPONENTS REPLACE "\\.cpp$" "")

rtype_select_plugins(SYSTEM_PLUGINS "${ORBYTE_SYSTEMS}" "${AVAILABLE_SYSTEMS}" "system")
rtype_select_plugins(COMPONENT_PLUGINS "${ORBYTE_COMPONENTS}" "${AVAILABLE_COMPONENTS}" "component")
# The engine keeps its plugin DLLs loaded while it compiles the game, and Windows can't rewrite
# a loaded DLL: there the game gets its own plugins, in <game>_plugins/ next to it
if(WIN32)
    set(GAME_PLUGINS_DIR "${ORBYTE_GAME_NAME}_plugins")
else()
    set(GAME_PLUGINS_DIR "plugins")
endif()

message(STATUS "Game binary: ${ORBYTE_GAME_NAME}")
message(STATUS "Game plugins: ${RTYPE_ROOT}/${GAME_PLUGINS_DIR}")
message(STATUS "Game systems: ${SYSTEM_PLUGINS}")
message(STATUS "Game components: ${COMPONENT_PLUGINS}")
message(STATUS "Game metrics: ${ORBYTE_METRICS}")

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
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
if(WIN32)
    target_compile_definitions(rtype_asio INTERFACE _WIN32_WINNT=0x0A00)
    target_link_libraries(rtype_asio INTERFACE ws2_32 mswsock)
endif()

# Lua is only needed by the Script system. It comes from the GitHub mirror: on Windows, CMake's
# downloader (schannel) fails the TLS handshake with www.lua.org.
if("Script" IN_LIST SYSTEM_PLUGINS)
    FetchContent_Declare(lua
        URL https://github.com/lua/lua/archive/refs/tags/v5.4.7.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(lua)

    # The repository keeps the sources at its root, next to the interpreter and test builds
    file(GLOB LUA_SOURCES "${lua_SOURCE_DIR}/*.c")
    list(REMOVE_ITEM LUA_SOURCES
        "${lua_SOURCE_DIR}/lua.c"
        "${lua_SOURCE_DIR}/luac.c"
        "${lua_SOURCE_DIR}/onelua.c"
        "${lua_SOURCE_DIR}/ltests.c"
    )
    add_library(lua_static STATIC ${LUA_SOURCES})
    set_target_properties(lua_static PROPERTIES LINKER_LANGUAGE C)
    target_include_directories(lua_static SYSTEM PUBLIC "${lua_SOURCE_DIR}")
    if(UNIX)
        target_compile_definitions(lua_static PRIVATE LUA_USE_POSIX)
        target_link_libraries(lua_static PUBLIC m)
    endif()

    FetchContent_Declare(sol2
        GIT_REPOSITORY https://github.com/ThePhD/sol2.git
        GIT_TAG v3.5.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(sol2)

    add_library(rtype_lua INTERFACE)
    target_link_libraries(rtype_lua INTERFACE lua_static sol2::sol2)
    # The repository has no lua.hpp: without this sol2 would pick the system one, and the
    # system's Lua (5.5 on recent distributions) instead of this 5.4
    target_compile_definitions(rtype_lua INTERFACE
        SOL_NO_LUA_HPP=1
        $<$<CONFIG:Debug>:SOL_ALL_SAFETIES_ON=1>
    )
endif()

# The audio library plugin is only needed by the Audio system
if("Audio" IN_LIST SYSTEM_PLUGINS)
    set(RTYPE_GAME_AUDIO ON)
else()
    set(RTYPE_GAME_AUDIO OFF)
endif()

set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO ${RTYPE_GAME_AUDIO} CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
if(WIN32)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()
if(CMAKE_CROSSCOMPILING AND MINGW)
    list(APPEND CMAKE_FIND_ROOT_PATH "${FETCHCONTENT_BASE_DIR}/sfml-src")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(RTYPE_SFML_EXTLIBS "${FETCHCONTENT_BASE_DIR}/sfml-src/extlibs/libs-mingw/x64")
    else()
        set(RTYPE_SFML_EXTLIBS "${FETCHCONTENT_BASE_DIR}/sfml-src/extlibs/libs-mingw/x86")
    endif()
    set(OPENAL_LIBRARY "${RTYPE_SFML_EXTLIBS}/libopenal32.a" CACHE FILEPATH "")
endif()
FetchContent_Declare(sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(sfml)

# ---------------------------------------------------------------------------
# Engine
# ---------------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${RTYPE_ROOT}/src/GameEngine/*.cpp"
    "${RTYPE_ROOT}/src/Orbyte/*.cpp"
)
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "/src/GameEngine/(Systems/.*|Components/.*|main)\\.cpp$")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "/src/GameEngine/(client|server)/")
list(APPEND ENGINE_SOURCES "${RTYPE_ROOT}/src/plugins/DynamicLibrary.cpp")
add_library(rtype_engine STATIC ${ENGINE_SOURCES})
target_include_directories(rtype_engine PUBLIC
    "${RTYPE_ROOT}/include"
    "${RTYPE_ROOT}/include/GameEngine/ECS/Managers"
)
target_link_libraries(rtype_engine PUBLIC ${CMAKE_DL_LIBS} bitsery)
target_compile_definitions(rtype_engine PUBLIC "ORBYTE_PLUGINS_DIR=\"./${GAME_PLUGINS_DIR}\"")

# ---------------------------------------------------------------------------
# Plugins
# ---------------------------------------------------------------------------
if(WIN32)
    set(RTYPE_PLUGIN_SUFFIX ".dll")
else()
    set(RTYPE_PLUGIN_SUFFIX ".so")
endif()

function(rtype_configure_plugin target output_name output_dir)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${output_name}"
        PREFIX ""
        SUFFIX "${RTYPE_PLUGIN_SUFFIX}"
        POSITION_INDEPENDENT_CODE ON
        LIBRARY_OUTPUT_DIRECTORY "$<1:${output_dir}>"
        RUNTIME_OUTPUT_DIRECTORY "$<1:${output_dir}>"
    )
endfunction()

add_library(rtype_sfml MODULE "${RTYPE_ROOT}/src/plugins/SFML.cpp")
rtype_configure_plugin(rtype_sfml "SFML" "${RTYPE_ROOT}/${GAME_PLUGINS_DIR}")
target_include_directories(rtype_sfml PRIVATE "${RTYPE_ROOT}/include")
target_link_libraries(rtype_sfml PRIVATE bitsery sfml-graphics sfml-window sfml-system)
set(LIBRARY_TARGETS rtype_sfml)

if(RTYPE_GAME_AUDIO)
    add_library(rtype_sfml_audio MODULE "${RTYPE_ROOT}/src/plugins/SFMLAudio.cpp")
    rtype_configure_plugin(rtype_sfml_audio "SFMLAudio" "${RTYPE_ROOT}/${GAME_PLUGINS_DIR}")
    target_include_directories(rtype_sfml_audio PRIVATE "${RTYPE_ROOT}/include")
    target_link_libraries(rtype_sfml_audio PRIVATE sfml-audio sfml-system)
    # SFML's prebuilt libFLAC needs msvcrt's __iob_func, gone from the Universal CRT
    if(MINGW)
        include(CheckSymbolExists)
        check_symbol_exists(_UCRT "stdio.h" RTYPE_MINGW_UCRT)
        if(RTYPE_MINGW_UCRT)
            target_sources(rtype_sfml_audio PRIVATE "${RTYPE_ROOT}/src/plugins/UcrtIobFunc.c")
        endif()
    endif()
    list(APPEND LIBRARY_TARGETS rtype_sfml_audio)
endif()

set(SYSTEM_TARGETS)
foreach(system IN LISTS SYSTEM_PLUGINS)
    set(system_target "rtype_system_${system}")
    add_library(${system_target} MODULE "${RTYPE_ROOT}/src/GameEngine/Systems/${system}System.cpp")
    rtype_configure_plugin(${system_target} "${system}System" "${RTYPE_ROOT}/${GAME_PLUGINS_DIR}/systems")
    target_link_libraries(${system_target} PRIVATE rtype_engine)
    if(system STREQUAL "Script")
        target_link_libraries(${system_target} PRIVATE rtype_lua)
    endif()
    list(APPEND SYSTEM_TARGETS ${system_target})
endforeach()

set(COMPONENT_TARGETS)
foreach(component IN LISTS COMPONENT_PLUGINS)
    set(component_target "rtype_component_${component}")
    add_library(${component_target} MODULE "${RTYPE_ROOT}/src/GameEngine/Components/${component}.cpp")
    rtype_configure_plugin(${component_target} "${component}" "${RTYPE_ROOT}/${GAME_PLUGINS_DIR}/components")
    target_link_libraries(${component_target} PRIVATE rtype_engine)
    list(APPEND COMPONENT_TARGETS ${component_target})
endforeach()

# ---------------------------------------------------------------------------
# Game
# ---------------------------------------------------------------------------
file(GLOB_RECURSE SOLO_SOURCES CONFIGURE_DEPENDS
    "${RTYPE_ROOT}/src/GameEngine/client/*.cpp"
    "${RTYPE_ROOT}/src/GameEngine/server/*.cpp"
)

# The game carries the world it was compiled with, so it doesn't need the exported world to run
# and two games never share one. GameWorld.cpp is only rewritten when the world changed.
set(ORBYTE_GAME_WORLD "/tmp/game.obe" CACHE FILEPATH "World embedded in the game, where the engine exports it")
set(GAME_WORLD_SOURCE "${CMAKE_BINARY_DIR}/generated/GameWorld.cpp")
add_custom_target(rtype_game_world
    COMMAND ${CMAKE_COMMAND} "-DINPUT=${ORBYTE_GAME_WORLD}" "-DOUTPUT=${GAME_WORLD_SOURCE}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/EmbedWorld.cmake"
    BYPRODUCTS "${GAME_WORLD_SOURCE}"
    VERBATIM
)

add_executable(OrByte-gameClient ${SOLO_SOURCES} "${GAME_WORLD_SOURCE}")
add_dependencies(OrByte-gameClient rtype_game_world)
target_include_directories(OrByte-gameClient PRIVATE "${RTYPE_ROOT}/include")
target_link_libraries(OrByte-gameClient PRIVATE rtype_engine rtype_asio bitsery)
add_dependencies(OrByte-gameClient ${LIBRARY_TARGETS} ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})
set_target_properties(OrByte-gameClient PROPERTIES
    OUTPUT_NAME "${ORBYTE_GAME_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "$<1:${RTYPE_ROOT}>"
)
foreach(metric IN LISTS ORBYTE_METRICS)
    if(NOT metric MATCHES "^[A-Za-z0-9_]+$")
        message(FATAL_ERROR "Invalid metric '${metric}': use letters, digits and '_'")
    endif()
    string(TOUPPER "${metric}" metric_define)
    target_compile_definitions(OrByte-gameClient PRIVATE "ORBYTE_METRIC_${metric_define}")
endforeach()

# Only GamePlugins.cpp sees the plugin lists, so changing them rebuilds a single file
list(JOIN SYSTEM_PLUGINS "," GAME_SYSTEMS)
list(JOIN COMPONENT_PLUGINS "," GAME_COMPONENTS)
set_source_files_properties("${RTYPE_ROOT}/src/GameEngine/client/GamePlugins.cpp" PROPERTIES
    COMPILE_DEFINITIONS "ORBYTE_GAME_SYSTEMS=\"${GAME_SYSTEMS}\";ORBYTE_GAME_COMPONENTS=\"${GAME_COMPONENTS}\""
)

foreach(target IN ITEMS
    rtype_engine
    OrByte-gameClient
    ${LIBRARY_TARGETS}
    ${SYSTEM_TARGETS}
    ${COMPONENT_TARGETS}
)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8 /bigobj)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    if(WIN32)
        target_compile_definitions(${target} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    endif()
endforeach()

# MinGW: embed the C++ runtime so the game and its plugins run without the toolchain's DLLs
if(MINGW)
    foreach(target IN ITEMS OrByte-gameClient ${LIBRARY_TARGETS} ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})
        target_link_options(${target} PRIVATE -static)
    endforeach()
endif()

if(WIN32 AND RTYPE_GAME_AUDIO)
    # the audio plugin needs openal32.dll next to the game
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(RTYPE_OPENAL_DLL "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
    else()
        set(RTYPE_OPENAL_DLL "${sfml_SOURCE_DIR}/extlibs/bin/x86/openal32.dll")
    endif()
    add_custom_command(TARGET OrByte-gameClient POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RTYPE_OPENAL_DLL}" "$<TARGET_FILE_DIR:OrByte-gameClient>"
        VERBATIM
    )
endif()
