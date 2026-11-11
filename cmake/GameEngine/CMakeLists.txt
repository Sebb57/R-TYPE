# Standalone build of OrByte-gameClient only:
#   cmake -S cmake/GameEngine -B build-solo && cmake --build build-solo
cmake_minimum_required(VERSION 3.23)

project(RTypeSolo VERSION 1.0 DESCRIPTION "R-Type solo game engine" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

get_filename_component(RTYPE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

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

set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
if(WIN32)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()
if(CMAKE_CROSSCOMPILING AND MINGW)
    list(APPEND CMAKE_FIND_ROOT_PATH "${FETCHCONTENT_BASE_DIR}/sfml-src")
endif()
FetchContent_Declare(sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(sfml)

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
rtype_configure_plugin(rtype_sfml "SFML" "${RTYPE_ROOT}/plugins")
target_include_directories(rtype_sfml PRIVATE "${RTYPE_ROOT}/include")
target_link_libraries(rtype_sfml PRIVATE bitsery sfml-graphics sfml-window sfml-system)

set(SYSTEM_PLUGINS Collider Control Display Health Physic)
set(SYSTEM_TARGETS)
foreach(system IN LISTS SYSTEM_PLUGINS)
    set(system_target "rtype_system_${system}")
    add_library(${system_target} MODULE "${RTYPE_ROOT}/src/GameEngine/Systems/${system}System.cpp")
    rtype_configure_plugin(${system_target} "${system}System" "${RTYPE_ROOT}/plugins/systems")
    target_link_libraries(${system_target} PRIVATE rtype_engine)
    list(APPEND SYSTEM_TARGETS ${system_target})
endforeach()

set(COMPONENT_PLUGINS
    AddHealth Animator Camera Collider Controllable Health Orbit Owner Poison
    RegenHealth RemoveHealth Sinus Spiral Sprite Transform Velocity Wither Name
)
set(COMPONENT_TARGETS)
foreach(component IN LISTS COMPONENT_PLUGINS)
    set(component_target "rtype_component_${component}")
    add_library(${component_target} MODULE "${RTYPE_ROOT}/src/GameEngine/Components/${component}.cpp")
    rtype_configure_plugin(${component_target} "${component}" "${RTYPE_ROOT}/plugins/components")
    target_link_libraries(${component_target} PRIVATE rtype_engine)
    list(APPEND COMPONENT_TARGETS ${component_target})
endforeach()

file(GLOB_RECURSE SOLO_SOURCES CONFIGURE_DEPENDS
    "${RTYPE_ROOT}/src/GameEngine/client/*.cpp"
    "${RTYPE_ROOT}/src/GameEngine/server/*.cpp"
)
add_executable(OrByte-gameClient ${SOLO_SOURCES})
target_include_directories(OrByte-gameClient PRIVATE "${RTYPE_ROOT}/include")
target_link_libraries(OrByte-gameClient PRIVATE rtype_engine rtype_asio bitsery)
add_dependencies(OrByte-gameClient rtype_sfml ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})
set_target_properties(OrByte-gameClient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${RTYPE_ROOT}>"
)

foreach(target IN ITEMS
    rtype_engine
    rtype_sfml
    OrByte-gameClient
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

if(MINGW)
    target_link_options(OrByte-gameClient PRIVATE -static)
endif()
