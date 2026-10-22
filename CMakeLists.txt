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

add_library(rtype-protocol STATIC
    src/server/net/TcpProtocol.cpp
    src/server/net/UdpProtocol.cpp
)
target_include_directories(rtype-protocol PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(rtype-protocol PUBLIC rtype_asio bitsery)
target_compile_features(rtype-protocol PUBLIC cxx_std_23)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GameEngine/*.cpp"
)
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "/src/GameEngine/(Systems/.*|Components/.*|main)\\.cpp$")
list(APPEND ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/DynamicLibrary.cpp")
add_library(r-type_engine STATIC ${ENGINE_SOURCES})
target_include_directories(r-type_engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/GameEngine/ECS/Managers"
)
target_link_libraries(r-type_engine PUBLIC ${CMAKE_DL_LIBS})
target_compile_features(r-type_engine PUBLIC cxx_std_23)

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
target_link_libraries(r-type_server PRIVATE r-type_engine rtype-protocol)
add_dependencies(r-type_server ${SYSTEM_TARGETS})
target_compile_features(r-type_server PRIVATE cxx_std_23)
set_target_properties(r-type_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

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

add_library(rtype_sfml MODULE src/plugins/SFML.cpp)
set_target_properties(rtype_sfml PROPERTIES
    OUTPUT_NAME "SFML"
    PREFIX ""
    SUFFIX ".so"
    POSITION_INDEPENDENT_CODE ON
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"
)
target_include_directories(rtype_sfml PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(rtype_sfml PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

target_compile_features(rtype_sfml PRIVATE cxx_std_23)

set(SYSTEM_PLUGINS Collider Display Health Physic)
set(SYSTEM_TARGETS)
foreach(system IN LISTS SYSTEM_PLUGINS)
    set(system_target "rtype_system_${system}")
    add_library(${system_target} MODULE "src/GameEngine/Systems/${system}System.cpp")
    set_target_properties(${system_target} PROPERTIES
        OUTPUT_NAME "${system}System"
        PREFIX ""
        SUFFIX ".so"
        POSITION_INDEPENDENT_CODE ON
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins/systems"
    )
    target_link_libraries(${system_target} PRIVATE r-type_engine)
    target_compile_features(${system_target} PRIVATE cxx_std_23)
    list(APPEND SYSTEM_TARGETS ${system_target})
endforeach()

set(COMPONENT_PLUGINS
    AddHealth Animator Collider Health Orbit Owner Poison RegenHealth
    RemoveHealth Sinus Spiral Sprite Transform Velocity Wither
)
set(COMPONENT_TARGETS)
foreach(component IN LISTS COMPONENT_PLUGINS)
    set(component_target "rtype_component_${component}")
    add_library(${component_target} MODULE "src/GameEngine/Components/${component}.cpp")
    set_target_properties(${component_target} PROPERTIES
        OUTPUT_NAME "${component}"
        PREFIX ""
        SUFFIX ".so"
        POSITION_INDEPENDENT_CODE ON
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins/components"
    )
    target_link_libraries(${component_target} PRIVATE r-type_engine)
    target_compile_features(${component_target} PRIVATE cxx_std_23)
    list(APPEND COMPONENT_TARGETS ${component_target})
endforeach()

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/client/*.cpp"
)
add_executable(r-type_client ${CLIENT_SOURCES})
target_include_directories(r-type_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(r-type_client PRIVATE r-type_engine rtype-protocol sfml-audio)

add_dependencies(r-type_client rtype_sfml ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})

target_compile_features(r-type_client PRIVATE cxx_std_23)

set_target_properties(r-type_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

add_executable(r-type_game-engine src/GameEngine/main.cpp)

target_link_libraries(r-type_game-engine PRIVATE r-type_engine)
add_dependencies(r-type_game-engine rtype_sfml ${SYSTEM_TARGETS} ${COMPONENT_TARGETS})

target_compile_features(r-type_game-engine PRIVATE cxx_std_23)

set_target_properties(r-type_game-engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

foreach(target IN ITEMS
    r-type_engine
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