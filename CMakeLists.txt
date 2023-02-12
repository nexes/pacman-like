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

# So we can pull down FTXUI
include(FetchContent)
FetchContent_Declare(
	ftxui
	GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
)
FetchContent_MakeAvailable(ftxui)

add_executable(Game src/main.cpp src/pacman.cpp)

target_include_directories(Game PRIVATE include)

target_link_libraries(
    Game
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)