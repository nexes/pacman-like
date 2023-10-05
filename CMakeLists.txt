cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_BUILD_TYPE Debug)

project(
    Team404_Game
    VERSION 1.0
    LANGUAGES CXX
    DESCRIPTION "Network game for UW CSS 432"
)

# for c++ Threads
find_package(Threads REQUIRED)

# So we can pull down FTXUI
include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG v4.0.0
)
FetchContent_MakeAvailable(ftxui)

# the files needed to make the client side binary
add_executable(
    Game
    src/main.cpp
    src/pacman.cpp
    src/ui.cpp
    src/client_connection.cpp
    src/serializer.cpp
)

# the files needed to make the server side binary
add_executable(
    Server
    src/server.cpp
    src/server_connection.cpp
    src/serializer.cpp
)

target_include_directories(Game PRIVATE include)

target_link_libraries(Server Threads::Threads)

# needed for the FTXUI library to work. This is per their documentation
target_link_libraries(
    Game
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)