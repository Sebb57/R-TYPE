cmake_minimum_required(VERSION 3.28)

project(RType VERSION 1.0 DESCRIPTION "R-Type multiplayer game" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
if(WIN32)
    target_compile_definitions(rtype_asio INTERFACE _WIN32_WINNT=0x0A00)
    target_link_libraries(rtype_asio INTERFACE ws2_32 mswsock)
endif()

# ---------------------------------------------------------------------------
# Lua (interpreter, built from source) + sol2 (header-only C++ binding)
# ---------------------------------------------------------------------------
FetchContent_Declare(lua
    URL https://www.lua.org/ftp/lua-5.4.7.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(lua)

file(GLOB LUA_SOURCES "${lua_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM LUA_SOURCES
    "${lua_SOURCE_DIR}/src/lua.c"
    "${lua_SOURCE_DIR}/src/luac.c"
)
add_library(lua_static STATIC ${LUA_SOURCES})
set_target_properties(lua_static PROPERTIES LINKER_LANGUAGE C)
target_include_directories(lua_static SYSTEM PUBLIC "${lua_SOURCE_DIR}/src")
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
target_compile_definitions(rtype_lua INTERFACE
    $<$<CONFIG:Debug>:SOL_ALL_SAFETIES_ON=1>
)

add_library(rtype-protocol STATIC
    src/server/net/TcpProtocol.cpp
    src/server/net/UdpProtocol.cpp
)
target_include_directories(rtype-protocol PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(rtype-protocol PUBLIC rtype_asio bitsery)
target_compile_features(rtype-protocol PUBLIC cxx_std_23)

# ---------------------------------------------------------------------------
# Engine (static library)
# ---------------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GameEngine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Orbyte/*.cpp"
)
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "/src/GameEngine/(Systems/.*|Components/.*|main)\\.cpp$")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "/src/GameEngine/(client|server)/")
list(APPEND ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/DynamicLibrary.cpp")
add_library(rtype_engine STATIC ${ENGINE_SOURCES})
target_include_directories(rtype_engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/GameEngine/ECS/Managers"
)
target_link_libraries(rtype_engine PUBLIC ${CMAKE_DL_LIBS} bitsery rtype_lua)
target_compile_features(rtype_engine PUBLIC cxx_std_23)
# Dev setup: scripts are read straight from the source tree, so saving a .lua
# file is picked up by hot reload without copying anything into build/
target_compile_definitions(rtype_engine PUBLIC
    RTYPE_SCRIPTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/scripts"
)

# ---------------------------------------------------------------------------
# Server
# ---------------------------------------------------------------------------
file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cxx"
)

list(REMOVE_ITEM SERVER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/net/TcpProtocol.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/net/UdpProtocol.cpp"
)

file(GLOB_RECURSE SERVER_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/server/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/server/*.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/server/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/server/*.hxx"
)

set(SERVER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
foreach(header IN LISTS SERVER_HEADERS)
    get_filename_component(header_dir "${header}" DIRECTORY)
    list(APPEND SERVER_INCLUDE_DIRS "${header_dir}")
endforeach()
list(REMOVE_DUPLICATES SERVER_INCLUDE_DIRS)

add_executable(r-type_server ${SERVER_SOURCES})
target_include_directories(r-type_server PRIVATE ${SERVER_INCLUDE_DIRS})
target_link_libraries(r-type_server PRIVATE rtype_engine rtype-protocol)
target_compile_features(r-type_server PRIVATE cxx_std_23)
set_target_properties(r-type_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>"
)

# ---------------------------------------------------------------------------
# SFML
# ---------------------------------------------------------------------------
set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
# On Windows, link SFML statically so the executables don't depend on sfml-*.dll.
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
    GIT_TAG 3.1.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(sfml)

# ---------------------------------------------------------------------------
# Plugins (runtime-loaded modules)
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
    target_compile_features(${target} PRIVATE cxx_std_23)
endfunction()

add_library(rtype_sfml MODULE src/plugins/SFML.cpp)
rtype_configure_plugin(rtype_sfml "SFML" "${CMAKE_SOURCE_DIR}/plugins")
target_include_directories(rtype_sfml PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(rtype_sfml PRIVATE
    bitsery
    SFML::Graphics
    SFML::Window
    SFML::System
)

# Audio library plugin: the only audio code that knows SFML (like rtype_sfml for graphics)
add_library(rtype_sfml_audio MODULE src/plugins/SFMLAudio.cpp)
rtype_configure_plugin(rtype_sfml_audio "SFMLAudio" "${CMAKE_SOURCE_DIR}/plugins")
target_include_directories(rtype_sfml_audio PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(rtype_sfml_audio PRIVATE
    SFML::Audio
    SFML::System
)

set(SYSTEM_PLUGINS Collider Control Display Health Physic Audio Script)
set(SYSTEM_TARGETS)
foreach(system IN LISTS SYSTEM_PLUGINS)
    set(system_target "rtype_system_${system}")
    add_library(${system_target} MODULE "src/GameEngine/Systems/${system}System.cpp")
    rtype_configure_plugin(${system_target} "${system}System" "${CMAKE_SOURCE_DIR}/plugins/systems")
    target_link_libraries(${system_target} PRIVATE rtype_engine)
    list(APPEND SYSTEM_TARGETS ${system_target})
endforeach()

set(COMPONENT_PLUGINS
    AddHealth Animator Camera Collider Controllable Health Orbit Owner Poison
    RegenHealth RemoveHealth Sinus Spiral Sprite Transform Velocity Wither Name Music Script
)
set(COMPONENT_TARGETS)
foreach(component IN LISTS COMPONENT_PLUGINS)
    set(component_target "rtype_component_${component}")
    add_library(${component_target} MODULE "src/GameEngine/Components/${component}.cpp")
    rtype_configure_plugin(${component_target} "${component}" "${CMAKE_SOURCE_DIR}/plugins/components")
    target_link_libraries(${component_target} PRIVATE rtype_engine)
    list(APPEND COMPONENT_TARGETS ${component_target})
endforeach()

add_dependencies(r-type_server ${SYSTEM_TARGETS})

# ---------------------------------------------------------------------------
# Client
# ---------------------------------------------------------------------------
file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/client/*.cpp"
)
add_executable(r-type_client ${CLIENT_SOURCES})
target_include_directories(r-type_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(r-type_client PRIVATE
    rtype_engine
    rtype-protocol
    SFML::Audio
)
add_dependencies(r-type_client rtype_sfml rtype_sfml_audio ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})
target_compile_features(r-type_client PRIVATE cxx_std_23)
set_target_properties(r-type_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>"
)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(RTYPE_OPENAL_DLL "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
    else()
        set(RTYPE_OPENAL_DLL "${sfml_SOURCE_DIR}/extlibs/bin/x86/openal32.dll")
    endif()
    add_custom_command(TARGET r-type_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RTYPE_OPENAL_DLL}" "$<TARGET_FILE_DIR:r-type_client>"
        VERBATIM
    )
endif()

# ---------------------------------------------------------------------------
# Standalone game engine executable
# ---------------------------------------------------------------------------
add_executable(r-type_game-engine src/GameEngine/main.cpp)
target_link_libraries(r-type_game-engine PRIVATE rtype_engine)
add_dependencies(r-type_game-engine rtype_sfml rtype_sfml_audio ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})
target_compile_features(r-type_game-engine PRIVATE cxx_std_23)
set_target_properties(r-type_game-engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>"
)
if(WIN32)
    # the audio plugin needs openal32.dll next to the executable
    add_custom_command(TARGET r-type_game-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RTYPE_OPENAL_DLL}" "$<TARGET_FILE_DIR:r-type_game-engine>"
        VERBATIM
    )
endif()

# ---------------------------------------------------------------------------
# Warnings
# ---------------------------------------------------------------------------
foreach(target IN ITEMS
    rtype_engine
    r-type_server
    r-type_client
    r-type_game-engine
    rtype_sfml
    rtype_sfml_audio
    ${SYSTEM_TARGETS}
    ${COMPONENT_TARGETS}
)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    endif()
endforeach()

# Windows: keep <windows.h> lean and stop its min/max macros from clashing with
# std::min/std::max. Applied to our targets only, SFML relies on the full header.
foreach(target IN ITEMS
    rtype-protocol
    rtype_engine
    rtype_sfml
    rtype_sfml_audio
    r-type_server
    r-type_client
    r-type_game-engine
    ${SYSTEM_TARGETS}
    ${COMPONENT_TARGETS}
)
    if(WIN32)
        target_compile_definitions(${target} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    endif()
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /bigobj)
    endif()
endforeach()

# MinGW: embed the C++ runtime so the .exe and plugin .dll files run without the
# toolchain's DLLs (libstdc++-6, libgcc_s_seh-1, libwinpthread-1).
if(MINGW)
    foreach(target IN ITEMS
        r-type_server
        r-type_client
        r-type_game-engine
        rtype_sfml
        rtype_sfml_audio
        ${SYSTEM_TARGETS}
        ${COMPONENT_TARGETS}
    )
        target_link_options(${target} PRIVATE -static)
    endforeach()
endif()
