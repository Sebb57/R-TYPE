cmake_minimum_required(VERSION 3.20)

project(RType
    VERSION 1.0
    DESCRIPTION "R-Type multiplayer game"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

set(BOOST_INCLUDE_LIBRARIES asio serialization CACHE STRING "" FORCE)
set(BOOST_ENABLE_CMAKE ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.85.0
    GIT_SHALLOW TRUE
)

set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE) # Boost::asio handles networking

FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(boost sfml)

                           
file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp")
add_library(rtype_common STATIC ${COMMON_SOURCES})
target_include_directories(rtype_common
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(rtype_common
    PUBLIC
        Boost::asio
        Boost::serialization
)
target_compile_features(rtype_common PUBLIC cxx_std_20)


file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/*.cpp")
add_library(rtype_engine STATIC ${ENGINE_SOURCES})
target_include_directories(rtype_engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(rtype_engine
    PUBLIC
        rtype_common
        Boost::asio
        Boost::serialization
)
target_compile_features(rtype_engine PUBLIC cxx_std_20)


file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.cpp")
add_library(rtype_game STATIC ${GAME_SOURCES})
target_include_directories(rtype_game
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(rtype_game
    PUBLIC
        rtype_engine
        sfml-graphics
        sfml-window
        sfml-audio
        sfml-system
)
target_compile_features(rtype_game PUBLIC cxx_std_20)


add_executable(r-type_client src/main.cpp)
target_include_directories(r-type_client
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(r-type_client
    PRIVATE
        rtype_game
)
target_compile_features(r-type_client PRIVATE cxx_std_20)

                              
file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cpp")
add_executable(r-type_server ${SERVER_SOURCES})
target_include_directories(r-type_server
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(r-type_server
    PRIVATE
        rtype_engine
)
target_compile_features(r-type_server PRIVATE cxx_std_20)

set(PROJECT_TARGETS rtype_common rtype_engine rtype_game r-type_client r-type_server)
foreach(target IN LISTS PROJECT_TARGETS)
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
