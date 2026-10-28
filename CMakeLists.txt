cmake_minimum_required(VERSION 3.23)

project(RType VERSION 1.0 DESCRIPTION "R-Type multiplayer game" LANGUAGES CXX)

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
# Systems/Components are built as plugins, main.cpp as its own executable.
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "/src/GameEngine/(Systems/.*|Components/.*|main)\\.cpp$")
# TEMPORARY (r-type_solo, see the end of this file): src/GameEngine/client and
# src/GameEngine/server hold the merged client/server model, not engine code.
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "/src/GameEngine/(client|server)/")
list(APPEND ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/DynamicLibrary.cpp")
add_library(rtype_engine STATIC ${ENGINE_SOURCES})
target_include_directories(rtype_engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/GameEngine/ECS/Managers"
)
target_link_libraries(rtype_engine PUBLIC ${CMAKE_DL_LIBS})
target_compile_features(rtype_engine PUBLIC cxx_std_23)

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
# Cross-compiling for Windows from Linux: SFML ships its dependencies prebuilt in
# extlibs/. Allow lookups there despite the toolchain confining them to the
# target sysroot, and point at OpenAL directly: FindOpenAL looks for "OpenAL32"
# but the file is "libopenal32.a", which only matches on a case-insensitive
# filesystem.
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
# Plugins (runtime-loaded modules)
# ---------------------------------------------------------------------------
if(WIN32)
    set(RTYPE_PLUGIN_SUFFIX ".dll")
else()
    set(RTYPE_PLUGIN_SUFFIX ".so")
endif()

# $<1:...> prevents multi-config generators (MSVC) from appending Debug/Release.
# On Windows a MODULE is a "runtime" artifact, so both properties are set.
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
    sfml-graphics
    sfml-window
    sfml-system
)

set(SYSTEM_PLUGINS Collider Control Display Health Physic)
set(SYSTEM_TARGETS)
foreach(system IN LISTS SYSTEM_PLUGINS)
    set(system_target "rtype_system_${system}")
    add_library(${system_target} MODULE "src/GameEngine/Systems/${system}System.cpp")
    rtype_configure_plugin(${system_target} "${system}System" "${CMAKE_SOURCE_DIR}/plugins/systems")
    target_link_libraries(${system_target} PRIVATE rtype_engine)
    list(APPEND SYSTEM_TARGETS ${system_target})
endforeach()

set(COMPONENT_PLUGINS
    AddHealth Animator Collider Controllable Health Orbit Owner Poison
    RegenHealth RemoveHealth Sinus Spiral Sprite Transform Velocity Wither
)
set(COMPONENT_TARGETS)
foreach(component IN LISTS COMPONENT_PLUGINS)
    set(component_target "rtype_component_${component}")
    add_library(${component_target} MODULE "src/GameEngine/Components/${component}.cpp")
    rtype_configure_plugin(${component_target} "${component}" "${CMAKE_SOURCE_DIR}/plugins/components")
    target_link_libraries(${component_target} PRIVATE rtype_engine)
    list(APPEND COMPONENT_TARGETS ${component_target})
endforeach()

# The server loads the systems at runtime: make sure they are built with it.
add_dependencies(r-type_server ${SYSTEM_TARGETS})

# ---------------------------------------------------------------------------
# Client
# ---------------------------------------------------------------------------
file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/client/*.cpp"
)
add_executable(r-type_client ${CLIENT_SOURCES})
target_include_directories(r-type_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(r-type_client PRIVATE rtype_engine rtype-protocol sfml-audio)
add_dependencies(r-type_client rtype_sfml ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})
target_compile_features(r-type_client PRIVATE cxx_std_23)
set_target_properties(r-type_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>"
)

# SFML is linked statically on Windows, but OpenAL stays a DLL (LGPL) and must
# sit next to the client executable.
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
add_dependencies(r-type_game-engine rtype_sfml ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})
target_compile_features(r-type_game-engine PRIVATE cxx_std_23)
set_target_properties(r-type_game-engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>"
)

# ---------------------------------------------------------------------------
# Warnings
# ---------------------------------------------------------------------------
foreach(target IN ITEMS
    rtype_engine
    r-type_server
    r-type_client
    r-type_game-engine
    rtype_sfml
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

# MinGW: embed the C++ runtime so the .exe files run without the toolchain's DLLs.
if(MINGW)
    foreach(target IN ITEMS r-type_server r-type_client r-type_game-engine)
        target_link_options(${target} PRIVATE -static)
    endforeach()
endif()

# =============================================================================
# TEMPORARY: merged client/server model for the future game engine
# -----------------------------------------------------------------------------
# r-type_solo is a client that hosts its own server to play solo, the network
# side only stays for spectating: the game is computed and rendered locally,
# spectators only receive snapshots of what to draw. It is the model the game
# engine will use, so it lives in src/GameEngine/{client,server} and
# include/GameEngine/{client,server,protocol}. It is fully separate from the
# old game above (r-type_client / r-type_server), which still builds as before.
#
# Remove this block and the ENGINE_SOURCES filter at the top of this file once
# the game engine builds this model itself.
# =============================================================================
file(GLOB_RECURSE SOLO_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GameEngine/client/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GameEngine/server/*.cpp")
add_executable(r-type_solo ${SOLO_SOURCES})
target_include_directories(r-type_solo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(r-type_solo PRIVATE rtype_engine rtype_asio bitsery)
add_dependencies(r-type_solo rtype_sfml ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})
target_compile_features(r-type_solo PRIVATE cxx_std_23)
set_target_properties(r-type_solo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>"
)
if(MSVC)
    target_compile_options(r-type_solo PRIVATE /W4 /permissive- /utf-8 /bigobj)
else()
    target_compile_options(r-type_solo PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(WIN32)
    target_compile_definitions(r-type_solo PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
if(MINGW)
    target_link_options(r-type_solo PRIVATE -static)
endif()
# =================== END OF TEMPORARY r-type_solo BLOCK ======================
